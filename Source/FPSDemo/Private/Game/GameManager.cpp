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

void UGameManager::Init()
{
	Super::Init();
	DsClient = MakeUnique<DedicatedServerClient>();
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
		Pickup->Destroy();
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
	UE_LOG(LogTemp, Log, TEXT("UGameManager::StartMatch: Starting match, traveling to game level"));
    // get match, room data
    URoomManager* RoomMgr = URoomManager::Get(GetWorld());
    // get current room data
    const FRoomData& RoomData = RoomMgr->GetCurrentRoomData();
    FString Options;

    if (!IsRunningDedicatedServer())
    {
        Options += TEXT("?listen"); // only for listen server (client-host)
    }

    if (IsRunningDedicatedServer())
    {
        Options += FString::Printf(TEXT("?DedicatedServer=1"));
	}
    else {
		Options += FString::Printf(TEXT("?DedicatedServer=0"));
    }
    if (RoomData.Mode == EMatchMode::Spike)
    {
		Options += FString::Printf(TEXT("?game=/Game/Main/Core/GM_Spike.GM_Spike_C"));
        UGameplayStatics::OpenLevel(this, FGameConstants::LEVEL_GHOST_MALL_MAP, true, Options);
        return;
    }
    else if (RoomData.Mode == EMatchMode::Zombie)
    {
        Options += FString::Printf(TEXT("?game=/Game/Main/Core/GM_Zombie.GM_Zombie_C"));
        UGameplayStatics::OpenLevel(this, FGameConstants::LEVEL_GHOST_MALL_MAP, true, Options);
        return;
    }
    UE_LOG(LogTemp, Warning, TEXT("Start Game Clicked"));
    UGameplayStatics::OpenLevel(
        this,
        FGameConstants::LEVEL_PLAYGROUND
    );
}

void UGameManager::InitFromGameLift(
    const FString& InRoomId,
    const FString& InMode,
    const FString& InToken)
{
	DsClient->SetBearerToken(InToken);
    RequestMatchDataAndStart();
}