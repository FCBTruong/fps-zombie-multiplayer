// Fill out your copyright notice in the Description page of Project Settings.


#include "Network/NetBoostrapSubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Lobby/RoomManager.h"


bool UNetBoostrapSubsystem::InitOSS()
{
    IOnlineSubsystem* OSS = IOnlineSubsystem::Get();
    if (!OSS) return false;

    Identity = OSS->GetIdentityInterface();
    Session = OSS->GetSessionInterface();

    return Identity.IsValid() && Session.IsValid();
}

void UNetBoostrapSubsystem::StartSelfHost(const FString& Map, const FString& InMatchId, const FString& InJoinKey)
{
	UE_LOG(LogTemp, Log, TEXT("DEBUG: StartSelfHost called"));
    bIsHost = true;
    MapName = Map;
    MatchId = InMatchId;
    JoinKey = InJoinKey;

    if (!InitOSS())
    {
        UE_LOG(LogTemp, Error, TEXT("DEBUG: InitOSS failed (OnlineSubsystem not available)"));
        return;
    }

    LoginDeviceId();
}

void UNetBoostrapSubsystem::JoinDedicated(const FString& ServerIpPort)
{
    if (APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr)
    {
        PC->ClientTravel(ServerIpPort, TRAVEL_Absolute);
    }
}

void UNetBoostrapSubsystem::LoginDeviceId()
{
	UE_LOG(LogTemp, Log, TEXT("DEBUG: LoginDeviceId called"));
    if (!Identity.IsValid()) return;

    LoginHandle = Identity->AddOnLoginCompleteDelegate_Handle(
        0,
        FOnLoginCompleteDelegate::CreateUObject(this, &UNetBoostrapSubsystem::OnLoginComplete)
    );

    FOnlineAccountCredentials Creds;
    Creds.Type = TEXT("deviceid"); // anonymous, no Epic account UI

	UE_LOG(LogTemp, Log, TEXT("DEBUG: Calling Identity->Login"));
    Identity->Login(0, Creds);
}

void UNetBoostrapSubsystem::OnLoginComplete(int32 LocalUserNum, bool bOk, const FUniqueNetId&, const FString&)
{
    Identity->ClearOnLoginCompleteDelegate_Handle(LocalUserNum, LoginHandle);
	UE_LOG(LogTemp, Log, TEXT("DEBUG: LoginComplete: %s"), bOk ? TEXT("OK") : TEXT("FAIL"));
    if (!bOk) return;

    if (bIsHost)
    {
        CreateMatchSession(); // bShouldAdvertise=true if clients find by filter
    }
    else
    {
        DoFindSessions();
    }
}

void UNetBoostrapSubsystem::CreateMatchSession()
{
    if (Session->GetNamedSession(NAME_GameSession))
    {
        DestroyHandle = Session->AddOnDestroySessionCompleteDelegate_Handle(
            FOnDestroySessionCompleteDelegate::CreateUObject(this, &UNetBoostrapSubsystem::OnDestroySessionComplete));

        Session->DestroySession(NAME_GameSession);
        return;
    }

    DoCreateSession();
}

void UNetBoostrapSubsystem::OnCreateSessionComplete(FName SessionName, bool bOk)
{
    Session->ClearOnCreateSessionCompleteDelegate_Handle(CreateHandle);
    UE_LOG(LogTemp, Log, TEXT("CreateSession: %s"), bOk ? TEXT("OK") : TEXT("FAIL"));
    if (!bOk) return;

    FNamedOnlineSession* NamedSession = Session->GetNamedSession(SessionName);
    if (!NamedSession)
    {
        UE_LOG(LogTemp, Error, TEXT("GetNamedSession failed"));
        return;
    }

    if (NamedSession->SessionInfo.IsValid())
    {
        FString SessionIdStr = NamedSession->SessionInfo->GetSessionId().ToString();
        UE_LOG(LogTemp, Log, TEXT("SessionId: %s"), *SessionIdStr);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("SessionInfo invalid"));
    }

    RegisterPostLoadMapDelegate();
    OpenListen();
}

void UNetBoostrapSubsystem::DoCreateSession()
{
    FOnlineSessionSettings S;
    S.bIsLANMatch = false;
    S.bShouldAdvertise = true;
    S.bUsesPresence = false;
    S.NumPublicConnections = 10;
    S.bAllowJoinInProgress = true;
    S.bUseLobbiesIfAvailable = false;

    S.Set(TEXT("matchId"), MatchId, EOnlineDataAdvertisementType::ViaOnlineService);
    S.Set(TEXT("joinKey"), JoinKey, EOnlineDataAdvertisementType::ViaOnlineService);

    CreateHandle = Session->AddOnCreateSessionCompleteDelegate_Handle(
        FOnCreateSessionCompleteDelegate::CreateUObject(this, &UNetBoostrapSubsystem::OnCreateSessionComplete));

    Session->CreateSession(0, NAME_GameSession, S);
}

void UNetBoostrapSubsystem::OnDestroySessionComplete(FName SessionName, bool bSuccess)
{
    Session->ClearOnDestroySessionCompleteDelegate_Handle(DestroyHandle);

    if (!bSuccess)
    {
        UE_LOG(LogTemp, Error, TEXT("DestroySession failed"));
        return;
    }

    DoCreateSession();
}

void UNetBoostrapSubsystem::RegisterPostLoadMapDelegate()
{
    PostLoadHandle =
        FCoreUObjectDelegates::PostLoadMapWithWorld.AddUObject(
            this, &UNetBoostrapSubsystem::OnHostMapLoaded);
}

void UNetBoostrapSubsystem::OnHostMapLoaded(UWorld* LoadedWorld)
{
    FCoreUObjectDelegates::PostLoadMapWithWorld.Remove(PostLoadHandle);

    if (!bIsHost || LoadedWorld != GetWorld())
        return;

    if (UGameInstance* GI = LoadedWorld->GetGameInstance())
    {
        if (URoomManager* RM = GI->GetSubsystem<URoomManager>())
        {
            RM->NotifySelfHostReady(); 
        }
    }
}

void UNetBoostrapSubsystem::OpenListen()
{
    if (MapName.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("MapName empty, defaulting to FPSMap"));
        MapName = TEXT("FPSMap");
    }

    UGameplayStatics::OpenLevel(GetWorld(), FName(*MapName), true, TEXT("listen"));
}

void UNetBoostrapSubsystem::OnJoinComplete(FName, EOnJoinSessionCompleteResult::Type Result)
{
    UE_LOG(LogTemp, Log, TEXT("JoinSessionComplete: %d"), (int32)Result);
    Session->ClearOnJoinSessionCompleteDelegate_Handle(JoinHandle);

    if (!Session.IsValid()) return;

    FString Addr;
    if (Session->GetResolvedConnectString(NAME_GameSession, Addr))
    {
        if (APlayerController* PC = GetWorld() ? GetWorld()->GetFirstPlayerController() : nullptr)
        {
            PC->ClientTravel(Addr, TRAVEL_Absolute);
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("GetResolvedConnectString failed"));
    }
}

void UNetBoostrapSubsystem::JoinSelfHostByMatch(const FString& InMatchId, const FString& InJoinKey)
{
	MatchId = InMatchId;
	JoinKey = InJoinKey;

    UE_LOG(LogTemp, Log, TEXT("DEBUG: JoinSelfHostByMatch called. MatchId: %s, JoinKey: %s"),
        *MatchId,
        *JoinKey
    );
	bool bInit = InitOSS();
    if (!bInit)
    {
        UE_LOG(LogTemp, Error, TEXT("DEBUG: InitOSS failed (OnlineSubsystem not available)"));
        return;
	}

    if (Identity->GetLoginStatus(0) != ELoginStatus::LoggedIn)
    {
        LoginDeviceId();    
        return;
    }

    DoFindSessions();
}

void UNetBoostrapSubsystem::DoFindSessions() {
    UE_LOG(
        LogTemp,
        Log,
        TEXT("FindSession params | MatchId: %s | JoinKey: %s"),
        *MatchId,
        *JoinKey
    );

    Search = MakeShared<FOnlineSessionSearch>();
    Search->bIsLanQuery = false;
    Search->MaxSearchResults = 10;
    Search->QuerySettings.Set(TEXT("matchId"), MatchId, EOnlineComparisonOp::Equals);
    Search->QuerySettings.Set(TEXT("joinKey"), JoinKey, EOnlineComparisonOp::Equals); 

    if (!Session.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("SessionInterface is invalid"));
        return;
    }

    FindHandle = Session->AddOnFindSessionsCompleteDelegate_Handle(
        FOnFindSessionsCompleteDelegate::CreateUObject(this, &UNetBoostrapSubsystem::OnFindSessionsComplete));

    bool bStarted = Session->FindSessions(0, Search.ToSharedRef());


    UE_LOG(LogTemp, Log, TEXT("FindSessions started: %d"), bStarted);

    if (!bStarted)
    {
        Session->ClearOnFindSessionsCompleteDelegate_Handle(FindHandle);
    }

}

void UNetBoostrapSubsystem::OnFindSessionsComplete(bool bOk)
{
    Session->ClearOnFindSessionsCompleteDelegate_Handle(FindHandle);
    if (!bOk || !Search.IsValid() || Search->SearchResults.Num() == 0)
    {
        UE_LOG(LogTemp, Error, TEXT("No session found for match"));
        return;
    }
    UE_LOG(LogTemp, Log, TEXT("Find session okk..."));

    JoinHandle = Session->AddOnJoinSessionCompleteDelegate_Handle(
        FOnJoinSessionCompleteDelegate::CreateUObject(this, &UNetBoostrapSubsystem::OnJoinComplete));

    Session->JoinSession(0, NAME_GameSession, Search->SearchResults[0]);
}