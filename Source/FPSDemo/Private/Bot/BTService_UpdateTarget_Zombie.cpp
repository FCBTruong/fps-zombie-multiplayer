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

    ABotAIController* AICon = Cast<ABotAIController>(OwnerComp.GetAIOwner());
    if (!AICon) return;

    UAIPerceptionComponent* Perception = AICon->GetAIPerceptionComponent();
    APawn* SelfPawn = AICon->GetPawn();
	ABaseCharacter* SelfCharacter = Cast<ABaseCharacter>(SelfPawn);
    if (!Perception || !SelfPawn) return;

    const bool bSelfIsZombie = SelfCharacter->IsCharacterRole(ECharacterRole::Zombie);

    ABaseCharacter* CurrentTargetChar = AICon->GetTargetActor();
    TArray<AActor*> PerceivedActors;
    Perception->GetCurrentlyPerceivedActors(UAISense_Sight::StaticClass(), PerceivedActors);
 
    ABaseCharacter* BestTarget = nullptr;
    FindBestTarget(PerceivedActors, SelfCharacter, BestTarget);

    if (BestTarget == nullptr) {
        if (CurrentTargetChar) {
            if (CurrentTargetChar->IsDead()) {
                AICon->SetTargetActor(nullptr);
                AICon->ClearFocus(EAIFocusPriority::Gameplay);
                return;
            }
            if (GetWorld()->GetTimeSeconds() - AICon->GetLastTimeSeenTarget() < 2) {
                // allow memory
                AICon->ClearFocus(EAIFocusPriority::Gameplay);
                return;
            }
        }
    }
   
    AICon->SetTargetActor(BestTarget);

    if (BestTarget) {
        FVector AimLoc = BestTarget->GetActorLocation();
       
        AimLoc = BestTarget->GetAimPoint(EAimPointPolicy::HeadOrBody, 0.2f); // 50% head
        // Focus the point
        AICon->SetFocalPoint(AimLoc, EAIFocusPriority::Gameplay);
    }
    else {
        AICon->ClearFocus(EAIFocusPriority::Gameplay);
    }
}

void UBTService_UpdateTarget_Zombie::FindBestTarget(TArray<AActor*> PerceivedActors, ABaseCharacter* SelfPawn, ABaseCharacter*& OutBestTarget) {
    float BestDistSq = TNumericLimits<float>::Max();

    const FVector SelfLoc = SelfPawn->GetActorLocation();
	ETeamId MyTeamId = SelfPawn->GetTeamId();
	
    for (AActor* Actor : PerceivedActors)
    {
        if (!Actor || Actor == SelfPawn) continue;

        ABaseCharacter* TargetChar = Cast<ABaseCharacter>(Actor);

        if (!TargetChar) continue;

		if (!TargetChar->IsAlive()) continue;

		ETeamId TargetTeamId = TargetChar->GetTeamId();
		if (TargetTeamId == MyTeamId) continue; // Skip same team
        if (!SelfPawn->CanSeeThisActor(TargetChar)) continue;
        // Nearest
        const float DistSq = FVector::DistSquared(SelfLoc, Actor->GetActorLocation());
        if (DistSq < BestDistSq)
        {
            BestDistSq = DistSq;
            OutBestTarget = TargetChar;
        }
    }
}