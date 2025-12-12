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

UBTService_UpdateSpikeState::UBTService_UpdateSpikeState()
{
    UE_LOG(LogTemp, Log, TEXT("UBTService_UpdateSpikeState: Constructor called"));
    Interval = 0.5f;
    RandomDeviation = 0.f;
    bNotifyTick = true;
}

void UBTService_UpdateSpikeState::TickNode(
    UBehaviorTreeComponent& OwnerComp,
    uint8* NodeMemory,
    float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
    ABotAIController* AICon = Cast<ABotAIController>(OwnerComp.GetAIOwner());
    if (!AICon) return;

    //UE_LOG(LogTemp, Log, TEXT("UBTService_UpdateSpikeState: debug02 called"));
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();

	// check if player is near plant location
	APawn* AIPawn = AICon->GetPawn();
	if (AIPawn) {
		FVector PlantLocation = BB->GetValueAsVector("Vec_PlantLocation"); 
		float DistToPlant = FVector::Dist(AIPawn->GetActorLocation(), PlantLocation);
		bool IsInBombArea = (DistToPlant < 150.f);
		BB->SetValueAsBool("B_IsInBombArea", IsInBombArea);
	}
}