// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/AI/BTTask_RandomScoutLocation.h"
#include "Game/AI/BotAIController.h"
#include "Game/Subsystems/ActorManager.h"

EBTNodeResult::Type UBTTask_RandomScoutLocation::ExecuteTask(
    UBehaviorTreeComponent& OwnerComp,
    uint8* NodeMemory)
{
    ABotAIController* AI = Cast<ABotAIController>(OwnerComp.GetAIOwner());
    if (!AI)
    {
        return EBTNodeResult::Failed;
    }

    AActorManager* ActorMgr = AActorManager::Get(GetWorld());
    if (!ActorMgr)
    {
        return EBTNodeResult::Failed;
    }

	// get random scout location
	FVector ScoutLocation = ActorMgr->GetRandomScoutLocation();

	AI->SetScoutLocation(ScoutLocation);
    return EBTNodeResult::Succeeded;
}