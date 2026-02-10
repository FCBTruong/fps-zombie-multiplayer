// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/GameManager.h"
#include "Pickup/PickupItem.h"
#include "Game/ShooterGameState.h"
#include "Characters/BaseCharacter.h"
#include "Asset/CharacterAsset.h"
#include "Game/GlobalDataAsset.h"
#include "Kismet/GameplayStatics.h"
#include "Network/DedicatedServerClient.h"
#include "Lobby/RoomManager.h"
#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "OnlineSessionSettings.h"
#include "Lobby/PlayerInfoManager.h"

void UGameManager::Init()
{
	Super::Init();
	DsClient = MakeUnique<DedicatedServerClient>();
    FWorldDelegates::OnWorldCleanup.AddUObject(
        this,
        &UGameManager::OnWorldCleanup
    );
}

void UGameManager::OnWorldCleanup(
    UWorld* World,
    bool bSessionEnded,
    bool bCleanupResources) {
	PickupItemsOnMap.Empty();
}

void UGameManager::OnStart()
{
    Super::OnStart();
    CurrentPickupId = 1000;
}

FPickupData UGameManager::GetDataPickupItem(int32 ItemOnMapId) {
   
	FPickupData temp;
	temp.Id = -1;
	return temp;
}

void UGameManager::FindAndDestroyItemNode(int32 ItemOnMapId) {
	if (PickupItemsOnMap.Contains(ItemOnMapId)) {
        APickupItem* Pickup = PickupItemsOnMap[ItemOnMapId];
        if (Pickup) {
            Pickup->Destroy();
        }
        PickupItemsOnMap.Remove(ItemOnMapId);
	}
}

int32 UGameManager::GetNextItemOnMapId() {
    return CurrentPickupId++;
}

APickupItem* UGameManager::CreatePickupActor(FPickupData Data)
{
    APickupItem* Pickup = GetWorld()->SpawnActor<APickupItem>(
        APickupItem::StaticClass(),
        FVector::ZeroVector,
        FRotator::ZeroRotator
    );

    if (Pickup) {
        Pickup->SetData(Data);
        Pickup->GetItemMesh()->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
        Pickup->GetItemMesh()->SetEnableGravity(true);
        Pickup->GetItemMesh()->SetLinearDamping(0.8f);
        Pickup->GetItemMesh()->SetAngularDamping(0.8f);
        Pickup->GetItemMesh()->SetPhysicsMaxAngularVelocityInDegrees(720.f);

		Pickup->SetActorLocation(Data.Location);
		PickupItemsOnMap.Add(Data.Id, Pickup);


    }
    return Pickup;
}


void UGameManager::CleanPickupItemsOnMap()
{
    for (auto& Elem : PickupItemsOnMap) {
        APickupItem* Pickup = Elem.Value;
        if (IsValid(Pickup))
        {
            Pickup->Destroy();
        }
	}
    PickupItemsOnMap.Empty();
}

APickupItem* UGameManager::GetPickupNode(int PickupId) {
    if (PickupItemsOnMap.Contains(PickupId)) {
        return PickupItemsOnMap[PickupId];
	}
	return nullptr;
}

APickupItem* UGameManager::GetPickupSpike() const{
    if (PickupSpike.IsValid()) {
        return PickupSpike.Get();
	}
    return nullptr;
}

void UGameManager::SetPickupSpike(APickupItem* SpikeItem) {
    PickupSpike = SpikeItem;
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

void UGameManager::RegisterPlayer(ABaseCharacter* Pawn) {
    if (!Pawn) return;
	RegisteredPlayers.Add(Pawn);
}

void UGameManager::UnregisterPlayer(ABaseCharacter* Pawn) {
    if (!Pawn) return;
    RegisteredPlayers.Remove(Pawn);
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
			FRoomData RoomData;
            DedicatedServerClient::ParseMatchInfo(ResponseBody, RoomData);

			URoomManager* RoomMgr = URoomManager::Get(GetWorld());
            if (RoomMgr)
            {
				RoomMgr->SetCurrentRoomData(RoomData);
				StartMatch();
            }

            UE_LOG(LogTemp, Log, TEXT("UGameManager::RequestMatchDataAndStart: Received match info: %s"), *ResponseBody);
        }
    );
}

void UGameManager::StartMatch()
{
    URoomManager* RoomMgr = URoomManager::Get(GetWorld());
    const FRoomData& RoomData = RoomMgr->GetCurrentRoomData();

    PendingOptions.Empty();
    PendingMapName = FGameConstants::LEVEL_GHOST_MALL_MAP;

    if (RoomData.Mode == EMatchMode::Spike)
    {
        PendingOptions = TEXT("?listen?game=/Game/Main/Core/GM_Spike.GM_Spike_C");
    }
    else if (RoomData.Mode == EMatchMode::Zombie)
    {
        PendingOptions = TEXT("?listen?game=/Game/Main/Core/GM_Zombie.GM_Zombie_C");
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
