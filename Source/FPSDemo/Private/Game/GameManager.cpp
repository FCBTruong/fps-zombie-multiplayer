// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/GameManager.h"
#include "Game/Items/Pickup/PickupItem.h"
#include "Game/Framework/ShooterGameState.h"
#include "Game/Characters/BaseCharacter.h"
#include "Game/Data/CharacterAsset.h"
#include "Shared/Data/GlobalDataAsset.h"
#include "Kismet/GameplayStatics.h"
#include "Network/DedicatedServerClient.h"
#include "Game/Data/MatchInfo.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "Shared/System/PlayerInfoManager.h"

#if WITH_GAMELIFT
#include "GameLiftServerSDK.h"
#include "GameLiftServerSDKModels.h"
#endif

void UGameManager::Init()
{
	Super::Init();
	DsClient = MakeUnique<DedicatedServerClient>();

#if WITH_GAMELIFT
    InitGameLift();
#endif

    UE_LOG(LogTemp, Log, TEXT("GameLift InitSDK success"));
}

void UGameManager::OnStart()
{
    Super::OnStart();
}


void UGameManager::InitGameLift()
{
#if WITH_GAMELIFT
    UE_LOG(LogTemp, Log, TEXT("Calling InitGameLift..."));

    // Getting the module first.
    FGameLiftServerSDKModule* GameLiftSdkModule = &FModuleManager::LoadModuleChecked<FGameLiftServerSDKModule>(FName("GameLiftServerSDK"));

    //Define the server parameters for a GameLift Anywhere fleet. These are not needed for a GameLift managed EC2 fleet.
    FServerParameters ServerParametersForAnywhere;

    bool bIsAnywhereActive = false;
    if (FParse::Param(FCommandLine::Get(), TEXT("glAnywhere")))
    {
        bIsAnywhereActive = true;
    }

    if (bIsAnywhereActive)
    {
        UE_LOG(LogTemp, Log, TEXT("Configuring server parameters for Anywhere..."));

        // If GameLift Anywhere is enabled, parse command line arguments and pass them in the ServerParameters object.
        FString glAnywhereWebSocketUrl = "";
        if (FParse::Value(FCommandLine::Get(), TEXT("glAnywhereWebSocketUrl="), glAnywhereWebSocketUrl))
        {
            ServerParametersForAnywhere.m_webSocketUrl = TCHAR_TO_UTF8(*glAnywhereWebSocketUrl);
        }

        FString glAnywhereFleetId = "";
        if (FParse::Value(FCommandLine::Get(), TEXT("glAnywhereFleetId="), glAnywhereFleetId))
        {
            ServerParametersForAnywhere.m_fleetId = TCHAR_TO_UTF8(*glAnywhereFleetId);
        }

        FString glAnywhereProcessId = "";
        if (FParse::Value(FCommandLine::Get(), TEXT("glAnywhereProcessId="), glAnywhereProcessId))
        {
            ServerParametersForAnywhere.m_processId = TCHAR_TO_UTF8(*glAnywhereProcessId);
        }
        else
        {
            // If no ProcessId is passed as a command line argument, generate a randomized unique string.
            FString TimeString = FString::FromInt(std::time(nullptr));
            FString ProcessId = "ProcessId_" + TimeString;
            ServerParametersForAnywhere.m_processId = TCHAR_TO_UTF8(*ProcessId);
        }

        FString glAnywhereHostId = "";
        if (FParse::Value(FCommandLine::Get(), TEXT("glAnywhereHostId="), glAnywhereHostId))
        {
            ServerParametersForAnywhere.m_hostId = TCHAR_TO_UTF8(*glAnywhereHostId);
        }

        FString glAnywhereAuthToken = "";
        if (FParse::Value(FCommandLine::Get(), TEXT("glAnywhereAuthToken="), glAnywhereAuthToken))
        {
            ServerParametersForAnywhere.m_authToken = TCHAR_TO_UTF8(*glAnywhereAuthToken);
        }

        FString glAnywhereAwsRegion = "";
        if (FParse::Value(FCommandLine::Get(), TEXT("glAnywhereAwsRegion="), glAnywhereAwsRegion))
        {
            ServerParametersForAnywhere.m_awsRegion = TCHAR_TO_UTF8(*glAnywhereAwsRegion);
        }

        FString glAnywhereAccessKey = "";
        if (FParse::Value(FCommandLine::Get(), TEXT("glAnywhereAccessKey="), glAnywhereAccessKey))
        {
            ServerParametersForAnywhere.m_accessKey = TCHAR_TO_UTF8(*glAnywhereAccessKey);
        }

        FString glAnywhereSecretKey = "";
        if (FParse::Value(FCommandLine::Get(), TEXT("glAnywhereSecretKey="), glAnywhereSecretKey))
        {
            ServerParametersForAnywhere.m_secretKey = TCHAR_TO_UTF8(*glAnywhereSecretKey);
        }

        FString glAnywhereSessionToken = "";
        if (FParse::Value(FCommandLine::Get(), TEXT("glAnywhereSessionToken="), glAnywhereSessionToken))
        {
            ServerParametersForAnywhere.m_sessionToken = TCHAR_TO_UTF8(*glAnywhereSessionToken);
        }

        UE_LOG(LogTemp, SetColor, TEXT("%s"), COLOR_YELLOW);
        UE_LOG(LogTemp, Log, TEXT(">>>> WebSocket URL: %s"), *ServerParametersForAnywhere.m_webSocketUrl);
        UE_LOG(LogTemp, Log, TEXT(">>>> Fleet ID: %s"), *ServerParametersForAnywhere.m_fleetId);
        UE_LOG(LogTemp, Log, TEXT(">>>> Process ID: %s"), *ServerParametersForAnywhere.m_processId);
        UE_LOG(LogTemp, Log, TEXT(">>>> Host ID (Compute Name): %s"), *ServerParametersForAnywhere.m_hostId);
        UE_LOG(LogTemp, Log, TEXT(">>>> Auth Token: %s"), *ServerParametersForAnywhere.m_authToken);
        UE_LOG(LogTemp, Log, TEXT(">>>> Aws Region: %s"), *ServerParametersForAnywhere.m_awsRegion);
        UE_LOG(LogTemp, Log, TEXT(">>>> Access Key: %s"), *ServerParametersForAnywhere.m_accessKey);
        UE_LOG(LogTemp, Log, TEXT(">>>> Secret Key: %s"), *ServerParametersForAnywhere.m_secretKey);
        UE_LOG(LogTemp, Log, TEXT(">>>> Session Token: %s"), *ServerParametersForAnywhere.m_sessionToken);
        UE_LOG(LogTemp, SetColor, TEXT("%s"), COLOR_NONE);
    }

    UE_LOG(LogTemp, Log, TEXT("Initializing the GameLift Server..."));

    //InitSDK will establish a local connection with GameLift's agent to enable further communication.
    FGameLiftGenericOutcome InitSdkOutcome = GameLiftSdkModule->InitSDK(ServerParametersForAnywhere);
    if (InitSdkOutcome.IsSuccess())
    {
        UE_LOG(LogTemp, SetColor, TEXT("%s"), COLOR_GREEN);
        UE_LOG(LogTemp, Log, TEXT("GameLift InitSDK succeeded!"));
        UE_LOG(LogTemp, SetColor, TEXT("%s"), COLOR_NONE);
    }
    else
    {
        UE_LOG(LogTemp, SetColor, TEXT("%s"), COLOR_RED);
        UE_LOG(LogTemp, Log, TEXT("ERROR: InitSDK failed : ("));
        FGameLiftError GameLiftError = InitSdkOutcome.GetError();
        UE_LOG(LogTemp, Log, TEXT("ERROR: %s"), *GameLiftError.m_errorMessage);
        UE_LOG(LogTemp, SetColor, TEXT("%s"), COLOR_NONE);
        return;
    }

    ProcessParameters = MakeShared<FProcessParameters>();

    //When a game session is created, Amazon GameLift Servers sends an activation request to the game server and passes along the game session object containing game properties and other settings.
    //Here is where a game server should take action based on the game session object.
    //Once the game server is ready to receive incoming player connections, it should invoke GameLiftServerAPI.ActivateGameSession()
    ProcessParameters->OnStartGameSession.BindLambda([this, GameLiftSdkModule](Aws::GameLift::Server::Model::GameSession InGameSession)
        {
            FString GameSessionId = FString(InGameSession.GetGameSessionId());
            UE_LOG(LogTemp, Log, TEXT("GameSession Initializing: %s"), *GameSessionId);
            GameLiftSdkModule->ActivateGameSession();

            FString RoomId;
            FString Token = UTF8_TO_TCHAR(InGameSession.GetGameSessionData());

            int32 Count = 0;
            const Aws::GameLift::Server::Model::GameProperty* Properties = InGameSession.GetGameProperties(Count);

            for (int32 i = 0; i < Count; ++i)
            {
                const auto& Prop = Properties[i];
                const char* KeyC = Properties[i].GetKey();
                const char* ValueC = Properties[i].GetValue();

                const FString Key = UTF8_TO_TCHAR(KeyC);
                const FString Value = UTF8_TO_TCHAR(ValueC);

                if (Key == TEXT("roomId"))
                {
                    RoomId = Value;
                }
            }

            UE_LOG(LogTemp, Log, TEXT("GameSessionId: %s"), *GameSessionId);
            UE_LOG(LogTemp, Log, TEXT("RoomId: %s"), *RoomId);

            if (Token.IsEmpty())
            {
                UE_LOG(LogTemp, Error, TEXT("Missing dedicated server token"));
                return;
            }

            this->InitServerConfig(RoomId, Token);
            this->RequestMatchDataAndStart();
        });

    //OnProcessTerminate callback. Amazon GameLift Servers will invoke this callback before shutting down an instance hosting this game server.
    //It gives this game server a chance to save its state, communicate with services, etc., before being shut down.
    //In this case, we simply tell Amazon GameLift Servers we are indeed going to shutdown.
    ProcessParameters->OnTerminate.BindLambda([this]()
        {
            this->ShutdownGameLiftServer(true);
        });

    //This is the HealthCheck callback.
    //Amazon GameLift Servers will invoke this callback every 60 seconds or so.
    //Here, a game server might want to check the health of dependencies and such.
    //Simply return true if healthy, false otherwise.
    //The game server has 60 seconds to respond with its health status. Amazon GameLift Servers will default to 'false' if the game server doesn't respond in time.
    //In this case, we're always healthy!
    ProcessParameters->OnHealthCheck.BindLambda([]()
        {
            UE_LOG(LogTemp, Log, TEXT("Performing Health Check"));
            return true;
        });

    //GameServer.exe -port=7777 LOG=server.mylog
    ProcessParameters->port = FURL::UrlConfig.DefaultPort;
    TArray<FString> CommandLineTokens;
    TArray<FString> CommandLineSwitches;

    FCommandLine::Parse(FCommandLine::Get(), CommandLineTokens, CommandLineSwitches);

    for (FString SwitchStr : CommandLineSwitches)
    {
        FString Key;
        FString Value;

        if (SwitchStr.Split("=", &Key, &Value))
        {
            if (Key.Equals("port"))
            {
                ProcessParameters->port = FCString::Atoi(*Value);
            }
        }
    }

    //Here, the game server tells Amazon GameLift Servers where to find game session log files.
    //At the end of a game session, Amazon GameLift Servers uploads everything in the specified 
    //location and stores it in the cloud for access later.
    TArray<FString> Logfiles;
    Logfiles.Add(TEXT("GameLiftUnrealApp/Saved/Logs/server.log"));
    ProcessParameters->logParameters = Logfiles;

    //The game server calls ProcessReady() to tell Amazon GameLift Servers it's ready to host game sessions.
    UE_LOG(LogTemp, Log, TEXT("Calling Process Ready..."));
    FGameLiftGenericOutcome ProcessReadyOutcome = GameLiftSdkModule->ProcessReady(*ProcessParameters);

    if (ProcessReadyOutcome.IsSuccess())
    {
        UE_LOG(LogTemp, SetColor, TEXT("%s"), COLOR_GREEN);
        UE_LOG(LogTemp, Log, TEXT("Process Ready!"));
        UE_LOG(LogTemp, SetColor, TEXT("%s"), COLOR_NONE);
    }
    else
    {
        UE_LOG(LogTemp, SetColor, TEXT("%s"), COLOR_RED);
        UE_LOG(LogTemp, Log, TEXT("ERROR: Process Ready Failed!"));
        FGameLiftError ProcessReadyError = ProcessReadyOutcome.GetError();
        UE_LOG(LogTemp, Log, TEXT("ERROR: %s"), *ProcessReadyError.m_errorMessage);
        UE_LOG(LogTemp, SetColor, TEXT("%s"), COLOR_NONE);
    }

    UE_LOG(LogTemp, Log, TEXT("InitGameLift completed!"));
#endif
}


void UGameManager::ShutdownGameLiftServer(bool bSuccess)
{
#if WITH_GAMELIFT
    FGameLiftServerSDKModule* GameLiftSdkModule =
        &FModuleManager::LoadModuleChecked<FGameLiftServerSDKModule>(FName("GameLiftServerSDK"));

    const FGameLiftGenericOutcome ProcessEndingOutcome = GameLiftSdkModule->ProcessEnding();
    const FGameLiftGenericOutcome DestroyOutcome = GameLiftSdkModule->Destroy();

    if (!ProcessEndingOutcome.IsSuccess())
    {
        const FGameLiftError& Error = ProcessEndingOutcome.GetError();
        UE_LOG(LogTemp, Error, TEXT("ProcessEnding failed: %s"),
            Error.m_errorMessage.IsEmpty() ? TEXT("Unknown") : *Error.m_errorMessage);
    }

    if (!DestroyOutcome.IsSuccess())
    {
        const FGameLiftError& Error = DestroyOutcome.GetError();
        UE_LOG(LogTemp, Error, TEXT("Destroy failed: %s"),
            Error.m_errorMessage.IsEmpty() ? TEXT("Unknown") : *Error.m_errorMessage);
    }
#endif

    // This is what actually stops the UE dedicated server process
    FPlatformMisc::RequestExit(!bSuccess);
}

UGameManager* UGameManager::Get(UObject* WorldContextObject) {
    if (!WorldContextObject) {
        return nullptr;
    }
    UWorld* World = WorldContextObject->GetWorld();
    if (!World) {
        return nullptr;
    }
    UGameInstance* GI = World->GetGameInstance();
    if (!GI) {
        return nullptr;
    }
    UGameManager* GI_Cast = Cast<UGameManager>(GI);
	
    return GI_Cast;
}

void UGameManager::RequestMatchDataAndStart()
{
	UE_LOG(LogTemp, Log, TEXT("UGameManager::RequestMatchDataAndStart: Requesting match data from backend server"));
    // For dedicated server: request match data from backend server, then travel to game level
    if (!DsClient) {
        UE_LOG(LogTemp, Error, TEXT("UGameManager::RequestMatchDataAndStart: DsClient is null"));
        return;
	}
    DsClient->GetMatchInfo(
        [this](bool bOk, const FString& ResponseBody)
        {
            if (!bOk)
            {
                UE_LOG(LogTemp, Error, TEXT("UGameManager::RequestMatchDataAndStart: Failed to get match info"));
                return;
            }
			FMatchInfo MatchInfo;
            DedicatedServerClient::ParseMatchInfo(ResponseBody, MatchInfo);
		
			StartMatch(MatchInfo);

            UE_LOG(LogTemp, Log, TEXT("UGameManager::RequestMatchDataAndStart: Received match info: %s"), *ResponseBody);
        }
    );
}

void UGameManager::StartMatch(FMatchInfo MatchInfo)
{
	CurrentMatchInfo = MatchInfo;
    PendingOptions.Empty();
    PendingMapName = FGameConstants::LEVEL_GHOST_MALL_MAP;

    if (MatchInfo.Mode == EMatchMode::Spike)
    {
        PendingOptions = TEXT("?listen?game=/Game/Main/Data/GM_Spike.GM_Spike_C");
    }
    else if (MatchInfo.Mode == EMatchMode::Zombie)
    {
        PendingOptions = TEXT("?listen?game=/Game/Main/Data/GM_Zombie.GM_Zombie_C");
    }
    else if (MatchInfo.Mode == EMatchMode::DeathMatch)
    {
        PendingOptions = TEXT("?listen?game=/Game/Main/Data/GM_DeathMatch.GM_DeathMatch_C");
	}
    else
    {
        PendingMapName = FGameConstants::LEVEL_PLAYGROUND;
        PendingOptions = TEXT("?listen");
    }

    if (IsRunningDedicatedServer())
    {
        PendingOptions += TEXT("?DedicatedServer=1");
        UGameplayStatics::OpenLevel(this, PendingMapName, true, PendingOptions);
        return;
    }

    // self host
    int OwnerId = UPlayerInfoManager::Get(GetWorld())->GetUserId();;
    PendingOptions += FString::Printf(TEXT("?PlayerSessionId=%d"), OwnerId);

#if UE_BUILD_SHIPPING // offline mode, selfhost session currently not supported, directly open level without creating session
	UGameplayStatics::OpenLevel(this, PendingMapName, true, PendingOptions);
#else
    // Create session then travel in OnCreateSessionComplete
    CreateHostSession();
#endif
}


void UGameManager::InitServerConfig(
    const FString& InRoomId,
    const FString& InToken)
{
	DsClient->SetBearerToken(InToken);
}

void UGameManager::CreateHostSession()
{
    IOnlineSubsystem* OSS = IOnlineSubsystem::Get();
    if (!OSS) { UE_LOG(LogTemp, Error, TEXT("No OnlineSubsystem")); return; }

    IOnlineSessionPtr SI = OSS->GetSessionInterface();
    if (!SI.IsValid()) { UE_LOG(LogTemp, Error, TEXT("No SessionInterface")); return; }

    ULocalPlayer* LP = GetFirstGamePlayer();
    if (!LP) { UE_LOG(LogTemp, Error, TEXT("No LocalPlayer")); return; }

    FUniqueNetIdRepl NetId = LP->GetPreferredUniqueNetId();
    if (!NetId.IsValid()) { UE_LOG(LogTemp, Error, TEXT("Invalid NetId")); return; }

    // Bind once (store handle as member)
    OnCreateHandle = SI->AddOnCreateSessionCompleteDelegate_Handle(
        FOnCreateSessionCompleteDelegate::CreateUObject(this, &UGameManager::OnCreateSessionComplete)
    );

    FOnlineSessionSettings S;
    S.bIsLANMatch = true;
    S.bShouldAdvertise = true;
    S.NumPublicConnections = 10;

    S.bAllowJoinInProgress = true;
    S.bAllowJoinViaPresence = false;

    // If an old session exists, destroy first 
    if (SI->GetNamedSession(NAME_GameSession))
    {
        SI->DestroySession(NAME_GameSession);
    }

    SI->CreateSession(*NetId, NAME_GameSession, S);
}

void UGameManager::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	UE_LOG(LogTemp, Log, TEXT("OnCreateSessionComplete: %s, Success: %d"), *SessionName.ToString(), bWasSuccessful);
    IOnlineSessionPtr SI = IOnlineSubsystem::Get()->GetSessionInterface();
    if (SI.IsValid())
    {
        SI->ClearOnCreateSessionCompleteDelegate_Handle(OnCreateHandle);
    }

    if (!bWasSuccessful)
    {
        UE_LOG(LogTemp, Error, TEXT("CreateSession failed"));
        return;
    }

    UGameplayStatics::OpenLevel(this, PendingMapName, true, PendingOptions);
}
