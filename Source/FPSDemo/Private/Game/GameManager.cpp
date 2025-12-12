// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/GameManager.h"
#include "Pickup/PickupItem.h"
#include "Game/ShooterGameState.h"
#include "Weapons/WeaponDataManager.h"

int32 UGameManager::CurrentPickupId = 1000;

void UGameManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    UE_LOG(LogTemp, Warning, TEXT("ObjectAAA address = %p"), this);

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

APickupItem* UGameManager::GetPickupSpike() {
    UE_LOG(LogTemp, Warning, TEXT("PickupItemsOnMap count = %d"), PickupItemsOnMap.Num());
    for (const TPair<int32, APickupItem*>& Pair : PickupItemsOnMap)
    {
        APickupItem* Item = Pair.Value;
        if (!Item) continue;

        if (Item->GetData().ItemId == EItemId::SPIKE)   
        {
            return Item;
        }
    }
    return nullptr;
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
    return GI->GetSubsystem<UGameManager>();
}

void UGameManager::RegisterPlayer(ABaseCharacter* Pawn) {
    if (!Pawn) return;
	RegisteredPlayers.Add(Pawn);
}

void UGameManager::UnregisterPlayer(ABaseCharacter* Pawn) {
    if (!Pawn) return;
    RegisteredPlayers.Remove(Pawn);
}