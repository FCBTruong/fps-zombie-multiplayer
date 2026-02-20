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

void UGameManager::Init()
{
	Super::Init();
	DsClient = MakeUnique<DedicatedServerClient>();
}

void UGameManager::OnStart()
{
    Super::OnStart();
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
        PendingOptions = TEXT("?listen?game=/Game/Main/Core/GM_Spike.GM_Spike_C");
    }
    else if (MatchInfo.Mode == EMatchMode::Zombie)
    {
        PendingOptions = TEXT("?listen?game=/Game/Main/Core/GM_Zombie.GM_Zombie_C");
    }
    else if (MatchInfo.Mode == EMatchMode::DeathMatch)
    {
        PendingOptions = TEXT("?listen?game=/Game/Main/Core/GM_DeathMatch.GM_DeathMatch_C");
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

    // Create session then travel in OnCreateSessionComplete
    CreateHostSession();
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

    // self host
    int OwnerId = UPlayerInfoManager::Get(GetWorld())->GetUserId();;
    PendingOptions += FString::Printf(TEXT("?PlayerSessionId=%d"), OwnerId);

    UGameplayStatics::OpenLevel(this, PendingMapName, true, PendingOptions);
}
