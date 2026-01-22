#include "Network/DedicatedServerClient.h"
#include "Network/MyNetworkSettings.h"

DedicatedServerClient::DedicatedServerClient()
{
#if UE_BUILD_SHIPPING
    BaseUrl = GetDefault<UMyNetworkSettings>()->ProdRESTUrl;
#else
    BaseUrl = GetDefault<UMyNetworkSettings>()->DevRESTUrl;
#endif
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

void DedicatedServerClient::NotifyReady(const FString& ServerInstanceId, TFunction<void(bool bOk, const FString& ResponseBody)> Callback)
{
    // POST /ds/ready
    const FString Body = FString::Printf(TEXT("{\"serverInstanceId\":\"%s\"}"), *ServerInstanceId);
    SendJsonRequest(TEXT("/ds/ready"), TEXT("POST"), Body, MoveTemp(Callback));
}

void DedicatedServerClient::NotifyFinish(
    const FString& ServerInstanceId,
    const FString& EventId,
    const FString& ResultJson,
    TFunction<void(bool bOk, const FString& ResponseBody)> Callback)
{
    // POST /ds/finish
    // ResultJson should already be a JSON object string (e.g. {"winnerTeam":1})
    const FString Body = FString::Printf(
        TEXT("{\"serverInstanceId\":\"%s\",\"eventId\":\"%s\",\"result\":%s}"),
        *ServerInstanceId, *EventId, *ResultJson);

    SendJsonRequest(TEXT("/ds/finish"), TEXT("POST"), Body, MoveTemp(Callback));
}
