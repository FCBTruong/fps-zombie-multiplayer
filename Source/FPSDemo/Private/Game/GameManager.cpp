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
}

void UGameManager::GenItemNodesOnMap(const TArray<FPickupData>& Items)
{
    UWorld* World = GetWorld();
    if (!World) return;

    for (const FPickupData& Data : Items)
    {
        APickupItem* Node = World->SpawnActor<APickupItem>(
            APickupItem::StaticClass(),
            Data.Location,
            FRotator::ZeroRotator
        );
        Node->SetData(Data);
        ItemNodesOnMap.Add(Data.Id, Node);
    }

    UE_LOG(LogTemp, Log, TEXT("GenItemsOnMap %d items on map"), Items.Num());
}

FPickupData UGameManager::GetDataPickupItem(int32 ItemOnMapId) {
    AShooterGameState* GS = GetWorld()->GetGameState<AShooterGameState>();
    if (GS)
    {
        if (GS->ItemsOnMap.Contains(ItemOnMapId))
        {
            return GS->ItemsOnMap[ItemOnMapId];
        }
    }
	FPickupData temp;
	temp.Id = -1;
	return temp;
}

void UGameManager::FindAndDestroyItem(int32 ItemOnMapId) {
	// First, remove from game state
	AShooterGameState* GS = GetWorld()->GetGameState<AShooterGameState>();
    if (GS && GS->ItemsOnMap.Contains(ItemOnMapId))
    {
        GS->ItemsOnMap.Remove(ItemOnMapId);
	}
    if (AActor** ItemPtr = ItemNodesOnMap.Find(ItemOnMapId))
    {
        if (*ItemPtr)
        {
            (*ItemPtr)->Destroy();
        }
        ItemNodesOnMap.Remove(ItemOnMapId);
    }
}

UItemData * UGameManager::GetItemDataById(EItemId ItemId) {
    UWorld* World = GetWorld();
    if (!World) return nullptr;
    UGameInstance* GI = World->GetGameInstance();
    if (!GI) return nullptr;
    UWeaponDataManager* WeaponDataMgr = GI->GetSubsystem<UWeaponDataManager>();
    if (!WeaponDataMgr) return nullptr;
    return WeaponDataMgr->GetWeaponById(ItemId);
}

void UGameManager::OnReceivedItemsFromServer(const TArray<FPickupData>& Items)
{
    // First, clear existing items on the map
    for (auto& Pair : ItemNodesOnMap)
    {
        if (Pair.Value)
        {
            Pair.Value->Destroy();
        }
    }
    ItemNodesOnMap.Empty();
	// Update game state
	AShooterGameState* GS = GetWorld()->GetGameState<AShooterGameState>();
    if (GS) {
		GS->ItemsOnMap.Empty();
        for (const FPickupData& Data : Items) {
            GS->ItemsOnMap.Add(Data.Id, Data);
        }
    }

    // Now, generate new items based on the received data
    GenItemNodesOnMap(Items);
}

void UGameManager::OnNewItemDataSpawned(const TArray<FPickupData>& Items) {
    AShooterGameState* GS = GetWorld()->GetGameState<AShooterGameState>();
    if (GS) {
        for (const FPickupData& Data : Items) {
            GS->ItemsOnMap.Add(Data.Id, Data);
        }
    }
}

int32 UGameManager::GetNextItemOnMapId() {
    static int32 CurrentId = 1000; // Start from 1000 to avoid conflicts with predefined IDs
    return CurrentId++;
}

void UGameManager::OnNewItemNodeSpawned(AActor* Item, int32 OnMapId) {
    if (Item) {
        ItemNodesOnMap.Add(OnMapId, Item);
    }
}