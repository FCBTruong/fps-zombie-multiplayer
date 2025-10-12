// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/GameManager.h"
#include "Pickup/PickupItem.h"
#include "Game/ShooterGameState.h"

void UGameManager::GenItemsOnMap(const TArray<FPickupData>& Items)
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
		ItemsOnMap.Add(Data.Id, Node);
    }

    UE_LOG(LogTemp, Log, TEXT("GenItemsOnMap %d items on map"), Items.Num());
}

FPickupData UGameManager::GetDataPickupItem(int32 ItemOnMapId) {
    AShooterGameState* GS = GetWorld()->GetGameState<AShooterGameState>();
    if (GS)
    {
		return GS->ItemsOnMap[ItemOnMapId];
    }
	FPickupData temp;
	temp.Id = -1;
	return temp;
}

void UGameManager::FindAndDestroyItem(int32 ItemOnMapId) {
    if (AActor** ItemPtr = ItemsOnMap.Find(ItemOnMapId))
    {
        if (*ItemPtr)
        {
            (*ItemPtr)->Destroy();
        }
        ItemsOnMap.Remove(ItemOnMapId);
    }
}