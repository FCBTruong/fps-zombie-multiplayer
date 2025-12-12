// Fill out your copyright notice in the Description page of Project Settings.


#include "Bot/BTTask_PlantSpike.h"
#include "Controllers/BotAIController.h"

UBTTask_PlantSpike::UBTTask_PlantSpike()
{
    NodeName = "Task_PlantSpike";
    bNotifyTick = true;
    bNotifyTaskFinished = true;
}

EBTNodeResult::Type UBTTask_PlantSpike::ExecuteTask(
    UBehaviorTreeComponent& OwnerComp,
    uint8* NodeMemory)
{
    ABotAIController* AI = Cast<ABotAIController>(OwnerComp.GetAIOwner());
    if (!AI)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return EBTNodeResult::Failed;     // MUST return a value
    }

    // Start async planting animation / behavior
    AI->StartPlantingSpike();

    // Tell the BT we are running an async task
    return EBTNodeResult::InProgress;
}

void UBTTask_PlantSpike::TickTask(
    UBehaviorTreeComponent& OwnerComp,
    uint8* NodeMemory,
    float DeltaSeconds)
{
    
}