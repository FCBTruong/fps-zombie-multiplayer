// Fill out your copyright notice in the Description page of Project Settings.


#include "Bot/BTService_UpdateTarget_Zombie.h"
#include <Controllers/BotAIController.h>
#include "Characters/BaseCharacter.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Sight.h"

UBTService_UpdateTarget_Zombie::UBTService_UpdateTarget_Zombie()
{
    UE_LOG(LogTemp, Log, TEXT("UBTService_UpdateTarget_Zombie: Constructor called"));
    bNotifyTick = true;
}

void UBTService_UpdateTarget_Zombie::TickNode(
    UBehaviorTreeComponent& OwnerComp,
    uint8* NodeMemory,
    float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	UE_LOG(LogTemp, Log, TEXT("UBTService_UpdateTarget_Zombie: TickNode called"));

    ABotAIController* AICon = Cast<ABotAIController>(OwnerComp.GetAIOwner());
    if (!AICon) return;

    UAIPerceptionComponent* Perception = AICon->GetAIPerceptionComponent();
    APawn* SelfPawn = AICon->GetPawn();
	ABaseCharacter* SelfCharacter = Cast<ABaseCharacter>(SelfPawn);
    if (!Perception || !SelfPawn) return;

    const bool bSelfIsZombie = SelfCharacter->IsCharacterRole(ECharacterRole::Zombie);

    ABaseCharacter* CurrentTargetChar = AICon->GetTargetActor();
 
    // Perceived actors via Sight
    if (bSelfIsZombie) {
        TArray<AActor*> PerceivedActors;
        Perception->GetCurrentlyPerceivedActors(UAISense_Sight::StaticClass(), PerceivedActors);

        ABaseCharacter* BestTarget = nullptr;
        float BestDistSq = TNumericLimits<float>::Max();

        const FVector SelfLoc = SelfPawn->GetActorLocation();

        for (AActor* Actor : PerceivedActors)
        {
            if (!Actor || Actor == SelfPawn) continue;

			ABaseCharacter* TargetChar = Cast<ABaseCharacter>(Actor);

			if (!TargetChar) continue;
            
			const bool bTargetIsZombie = TargetChar->IsCharacterRole(ECharacterRole::Zombie);
			if (bTargetIsZombie) continue; // Skip other zombies

            // Nearest
            const float DistSq = FVector::DistSquared(SelfLoc, Actor->GetActorLocation());
            if (DistSq < BestDistSq)
            {
                BestDistSq = DistSq;
                BestTarget = TargetChar;
            }
        }

        // Write blackboard
        if (BestTarget) {
			AICon->SetTargetActor(Cast<ABaseCharacter>(BestTarget));
        }
        else {
            // No target found, keep current if valid
            if (CurrentTargetChar && !CurrentTargetChar->IsAlive()) {
				AICon->SetTargetActor(nullptr);
            }
		}
    }
}