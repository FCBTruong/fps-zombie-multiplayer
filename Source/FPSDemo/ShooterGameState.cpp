// Fill out your copyright notice in the Description page of Project Settings.


#include "ShooterGameState.h"
#include <Net/UnrealNetwork.h>
#include "PickupItem.h"

void AShooterGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
}


void AShooterGameState::Multicast_UpdateItemsOnMap_Implementation(const TArray<FPickupData>& NewItems)
{
    UWorld* World = GetWorld();
    if (!World) return;

    for (const FPickupData& Data : NewItems)
    {
        APickupItem* Node = World->SpawnActor<APickupItem>(
            APickupItem::StaticClass(),
            Data.Location,
            FRotator::ZeroRotator
        );
		Node->SetData(Data);
    }

    UE_LOG(LogTemp, Log, TEXT("UpdateItemsOnMap %d items on map"), NewItems.Num());
}
