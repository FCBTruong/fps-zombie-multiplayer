// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/AI/BTService_UpdateSpikeState.h"
#include "Game/AI/BotAIController.h"
#include "Game/Framework/MyPlayerState.h"
#include "Game/Framework/ShooterGameState.h"
#include <BehaviorTree/BlackboardComponent.h>
#include "Game/Modes/Spike/SpikeMode.h"
#include "Game/Characters/BaseCharacter.h"
#include "Game/GameManager.h"
#include "Game/Items/Pickup/PickupItem.h"
#include <Kismet/GameplayStatics.h>
#include "Game/Subsystems/ActorManager.h"
#include "Game/AI/BotRole.h"

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
