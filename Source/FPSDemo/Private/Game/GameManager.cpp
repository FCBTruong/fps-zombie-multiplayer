// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/GameManager.h"
#include "Pickup/PickupItem.h"
#include "Game/ShooterGameState.h"
#include "Weapons/WeaponDataManager.h"

void UGameManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    GlobalData = TSoftObjectPtr<UGlobalDataAsset>(
        FSoftObjectPath(TEXT("/Game/Main/Data/GlobalData.GlobalData"))
    ).LoadSynchronous();

	WeaponDataManager = GetWeaponDataManager();
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

UItemData * UGameManager::GetItemDataById(EItemId ItemId) {
	UWeaponDataManager* WeaponDataMgr = GetWeaponDataManager();
	if (!WeaponDataMgr) {
		UE_LOG(LogTemp, Warning, TEXT("GetItemDataById: WeaponData Manager is null"));
		return nullptr;
	}
    return WeaponDataMgr->GetWeaponById(ItemId);
}

UWeaponData* UGameManager::GetWeaponDataById(EItemId ItemId) {
    UWeaponDataManager* WeaponDataMgr = GetWeaponDataManager();
    if (!WeaponDataMgr) {
        UE_LOG(LogTemp, Warning, TEXT("GetWeaponDataById: WeaponData Manager is null"));
        return nullptr;
    }
    return WeaponDataMgr->GetWeaponById(ItemId);
}

UWeaponDataManager* UGameManager::GetWeaponDataManager() {
    if (WeaponDataManager) {
		UE_LOG(LogTemp, Warning, TEXT("GetWeaponDataManager: Returning cached WeaponDataManager"));
        return WeaponDataManager;
	}
    UWorld* World = GetWorld();
    if (!World) {
        return nullptr;
    }
    UGameInstance* GI = World->GetGameInstance();
    if (!GI) {
        return nullptr;
    }
    WeaponDataManager = GI->GetSubsystem<UWeaponDataManager>();
    return WeaponDataManager;
}

int32 UGameManager::GetNextItemOnMapId() {
    static int32 CurrentId = 1000; // Start from 1000 to avoid conflicts with predefined IDs
    return CurrentId++;
}

APickupItem* UGameManager::CreatePickupActor(FPickupData Data) {
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