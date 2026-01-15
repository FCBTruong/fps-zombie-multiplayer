// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/GameManager.h"
#include "Pickup/PickupItem.h"
#include "Game/ShooterGameState.h"
#include "Characters/BaseCharacter.h"
#include "Asset/CharacterAsset.h"
#include "Game/GlobalDataAsset.h"
#include "Data/MySaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "Utils/GameUtils.h"

void UGameManager::Init()
{
	Super::Init();
}

void UGameManager::OnStart()
{
    Super::OnStart();
    CurrentPickupId = 1000;
    LoadOrCreateLocalInfo();
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


void UGameManager::LoadOrCreateLocalInfo()
{
    using namespace PlayerLocalInfoConst;

    UMySaveGame* Save = nullptr;

    if (UGameplayStatics::DoesSaveGameExist(SLOT, USER_INDEX))
    {
        Save = Cast<UMySaveGame>(
            UGameplayStatics::LoadGameFromSlot(SLOT, USER_INDEX)
        );
    }

    if (!Save)
    {
        Save = Cast<UMySaveGame>(
            UGameplayStatics::CreateSaveGameObject(UMySaveGame::StaticClass())
        );
    }

    if (!Save)
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to create SaveGame object"));
        return;
    }

    if (Save->GuestId.IsEmpty())
    {
        Save->GuestId = GameUtils::GenerateMd5Token();
    }

    if (Save->PlayerName.IsEmpty())
    {
        Save->PlayerName = TEXT("Player");
    }

	GuestId = Save->GuestId;
	PlayerName = Save->PlayerName;

    UGameplayStatics::SaveGameToSlot(Save, SLOT, USER_INDEX);
}
