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
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
    ABotAIController* AICon = Cast<ABotAIController>(OwnerComp.GetAIOwner());
    if (!AICon) return;

    //UE_LOG(LogTemp, Log, TEXT("UBTService_UpdateSpikeState: debug02 called"));
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();

	// check if player is near plant location
	APawn* AIPawn = AICon->GetPawn();
	const EBotRole BotRole =
		static_cast<EBotRole>(BB->GetValueAsEnum(TEXT("E_Role")));
	
    switch (BotRole)
    {
        case EBotRole::A_FindSpike:
        {
			UE_LOG(LogTemp, Log, TEXT("UBTService_UpdateSpikeState: A_FindSpike called"));
			AActor* SpikeActor = UGameManager::Get(this->GetWorld())->GetPickupSpike();
            if (SpikeActor) {
				const FVector SpikeLocation = SpikeActor->GetActorLocation();
                BB->SetValueAsVector(TEXT("Vec_SpikeLocation"), SpikeLocation);
            }
            break;
        }
    }
}

void UBTService_UpdateSpikeState::OnBecomeRelevant(
    UBehaviorTreeComponent& OwnerComp,
    uint8* NodeMemory)
{
    Super::OnBecomeRelevant(OwnerComp, NodeMemory);
}
