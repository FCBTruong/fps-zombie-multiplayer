// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/ShooterGameState.h"
#include <Net/UnrealNetwork.h>
#include "Pickup/PickupItem.h"

AShooterGameState::AShooterGameState()
{
    // Enable replication
    bReplicates = true;
}

void AShooterGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    
}

TArray<FPickupData> AShooterGameState::GetItemsOnMap() const
{
    TArray<FPickupData> OutItems;
    ItemsOnMap.GenerateValueArray(OutItems);
    return OutItems;
}

