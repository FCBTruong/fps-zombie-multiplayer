// Fill out your copyright notice in the Description page of Project Settings.


#include "Bot/BTService_UpdateSpikeState.h"
#include "Controllers/BotAIController.h"
#include "Controllers/MyPlayerState.h"
#include "Game/ShooterGameState.h"
#include <BehaviorTree/BlackboardComponent.h>
#include "Game/SpikeMode.h"
#include "Characters/BaseCharacter.h"
#include "Game/GameManager.h"
#include "Pickup/PickupItem.h"
#include <Kismet/GameplayStatics.h>
#include "Game/ActorManager.h"
#include "Bot/BotRole.h"

UBTService_UpdateSpikeState::UBTService_UpdateSpikeState()
{
    UE_LOG(LogTemp, Log, TEXT("UBTService_UpdateSpikeState: Constructor called"));
    bNotifyTick = true;
}

void UBTService_UpdateSpikeState::TickNode(
    UBehaviorTreeComponent& OwnerComp,
    uint8* NodeMemory,
    float DeltaSeconds)
{
    return;
}

void UBTService_UpdateSpikeState::OnBecomeRelevant(
    UBehaviorTreeComponent& OwnerComp,
    uint8* NodeMemory)
{
    Super::OnBecomeRelevant(OwnerComp, NodeMemory);
}
