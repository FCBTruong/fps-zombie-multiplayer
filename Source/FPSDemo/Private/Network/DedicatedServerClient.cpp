#include "Network/DedicatedServerClient.h"
#include "Network/MyNetworkSettings.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

DedicatedServerClient::DedicatedServerClient()
{
#if UE_BUILD_SHIPPING
    BaseUrl = GetDefault<UMyNetworkSettings>()->ProdRESTUrl;
#else
    BaseUrl = GetDefault<UMyNetworkSettings>()->DevRESTUrl;
#endif
}

DedicatedServerClient::~DedicatedServerClient()
{
}

void DedicatedServerClient::SetBearerToken(const FString& InBearerToken)
{
    BearerToken = InBearerToken;
}

void DedicatedServerClient::ApplyCommonHeaders(const TSharedRef<IHttpRequest, ESPMode::ThreadSafe>& Req, const FString& Bearer)
{
    Req->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
    Req->SetHeader(TEXT("Accept"), TEXT("application/json"));
    if (!Bearer.IsEmpty())
    {
        Req->SetHeader(TEXT("Authorization"), FString::Printf(TEXT("Bearer %s"), *Bearer));
    }
}

void DedicatedServerClient::SendJsonRequest(
    const FString& Path,
    const FString& Verb,
    const FString& JsonBody,
    TFunction<void(bool bOk, const FString& ResponseBody)> Callback)
{
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Req = FHttpModule::Get().CreateRequest();

    const FString Url = BaseUrl.EndsWith(TEXT("/")) ? (BaseUrl.LeftChop(1) + Path) : (BaseUrl + Path);
    Req->SetURL(Url);
    Req->SetVerb(Verb);
	UE_LOG(LogTemp, Log, TEXT("DedicatedServerClient::SendJsonRequest: %s %s"), *Verb, *Url);

    ApplyCommonHeaders(Req, BearerToken);

    if (!JsonBody.IsEmpty())
    {
        Req->SetContentAsString(JsonBody);
    }

    Req->OnProcessRequestComplete().BindLambda(
        [Callback](FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
        {
            const bool bHasResponse = Response.IsValid();
            const int32 Code = bHasResponse ? Response->GetResponseCode() : 0;

            const bool bOk = bWasSuccessful && bHasResponse && Code >= 200 && Code < 300;
            const FString Body = bHasResponse ? Response->GetContentAsString() : TEXT("");

            if (Callback)
            {
                Callback(bOk, Body);
            }
        });

    Req->ProcessRequest();
}

void DedicatedServerClient::GetMatchInfo(TFunction<void(bool bOk, const FString& ResponseBody)> Callback)
{
    // GET /ds/match/info
    SendJsonRequest(TEXT("/ds/match/info"), TEXT("GET"), TEXT(""), MoveTemp(Callback));
}

void DedicatedServerClient::NotifyReady( TFunction<void(bool bOk, const FString& ResponseBody)> Callback)
{
	UE_LOG(LogTemp, Log, TEXT("DedicatedServerClient::NotifyReady"));
    // POST /ds/ready
    SendJsonRequest(TEXT("/ds/match/ready"), TEXT("POST"), {}, MoveTemp(Callback));
}

void DedicatedServerClient::NotifyFinish(
    const FString& ResultJson,
    TFunction<void(bool bOk, const FString& ResponseBody)> Callback)
{
    SendJsonRequest(TEXT("/ds/match/finish"), TEXT("POST"), ResultJson, MoveTemp(Callback));
}

bool DedicatedServerClient::ParseMatchInfo(const FString& JsonString, FRoomData& OutMatchInfo)
{
    TSharedPtr<FJsonObject> RootObj;
    const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(JsonString);

    if (!FJsonSerializer::Deserialize(Reader, RootObj) || !RootObj.IsValid())
    {
        return false;
    }

    // mode
    OutMatchInfo.Mode = static_cast<EMatchMode>(RootObj->GetIntegerField(TEXT("mode")));

    // players
    OutMatchInfo.Players.Reset();

    const TArray<TSharedPtr<FJsonValue>>* PlayersArrayPtr = nullptr;
    if (RootObj->TryGetArrayField(TEXT("players"), PlayersArrayPtr) && PlayersArrayPtr)
    {
        for (const TSharedPtr<FJsonValue>& Val : *PlayersArrayPtr)
        {
            const TSharedPtr<FJsonObject> PlayerObj = Val->AsObject();
            if (!PlayerObj.IsValid())
            {
                continue;
            }

            FPlayerRoomInfo Slot;
            Slot.PlayerId = PlayerObj->GetIntegerField(TEXT("userId"));

            // Use TryGet... to avoid crashes if fields are missing
            PlayerObj->TryGetStringField(TEXT("playerName"), Slot.PlayerName);
            PlayerObj->TryGetStringField(TEXT("avatar"), Slot.Avatar);

            bool bBot = false;
            if (PlayerObj->TryGetBoolField(TEXT("isBot"), bBot))
            {
                Slot.bIsBot = bBot;
            }

            OutMatchInfo.Players.Add(MoveTemp(Slot));
        }
    }

    return true;
}