// Fill out your copyright notice in the Description page of Project Settings.


#include "Bot/BTTask_RandomScoutLocation.h"
#include "Controllers/BotAIController.h"
#include "Game/ActorManager.h"

EBTNodeResult::Type UBTTask_RandomScoutLocation::ExecuteTask(
    UBehaviorTreeComponent& OwnerComp,
    uint8* NodeMemory)
{
	ABotAIController* AI = Cast<ABotAIController>(OwnerComp.GetAIOwner());

	AActorManager* ActorMgr = AActorManager::Get(this->GetWorld());

	// get random scout location
	FVector ScoutLocation = ActorMgr->GetRandomScoutLocation();

	AI->SetScoutLocation(ScoutLocation);
    return EBTNodeResult::Succeeded;
}