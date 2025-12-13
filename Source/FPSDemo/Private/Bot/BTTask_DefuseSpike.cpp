// Fill out your copyright notice in the Description page of Project Settings.


#include "Bot/BTTask_DefuseSpike.h"
#include "Controllers/BotAIController.h"
UBTTask_DefuseSpike::UBTTask_DefuseSpike() {

}

EBTNodeResult::Type UBTTask_DefuseSpike::ExecuteTask(
    UBehaviorTreeComponent& OwnerComp,
    uint8* NodeMemory)
{
    ABotAIController* AICon = Cast<ABotAIController>(OwnerComp.GetAIOwner());
    
    if (!AICon) {
        return EBTNodeResult::Failed;
	}

	UE_LOG(LogTemp, Log, TEXT("BTTask_DefuseSpike: Starting defuse spike task"));
	AICon->StartDefusingSpike();
    return EBTNodeResult::Succeeded;
}

