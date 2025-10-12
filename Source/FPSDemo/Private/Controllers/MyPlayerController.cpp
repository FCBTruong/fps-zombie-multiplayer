// Fill out your copyright notice in the Description page of Project Settings.


#include "Controllers/MyPlayerController.h"
#include "Pickup/PickupItem.h"
#include "Game/ShooterGameState.h"
#include "Game/GameManager.h"

void AMyPlayerController::Client_ReceiveItemsOnMap_Implementation(const TArray<FPickupData>& Items)
{
    UE_LOG(LogTemp, Warning, TEXT("Client_ReceiveItemsOnMap received %d items"), Items.Num());
    UGameManager* GMR = GetGameInstance()->GetSubsystem<UGameManager>();
    if (!GMR)
    {
        return;
    }
    GMR->GenItemsOnMap(Items);
}
