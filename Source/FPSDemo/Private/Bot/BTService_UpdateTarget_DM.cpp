// Fill out your copyright notice in the Description page of Project Settings.


#include "Bot/BTService_UpdateTarget_DM.h"

#include <Controllers/BotAIController.h>
#include "Characters/BaseCharacter.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISense_Sight.h"

UBTService_UpdateTarget_DM::UBTService_UpdateTarget_DM()
{
    UE_LOG(LogTemp, Log, TEXT("UBTService_UpdateTarget_DM: Constructor called"));
    bNotifyTick = true;
}

void UBTService_UpdateTarget_DM::TickNode(
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

    TArray<AActor*> PerceivedActors;
    Perception->GetCurrentlyPerceivedActors(UAISense_Sight::StaticClass(), PerceivedActors);

    ABaseCharacter* BestTarget = nullptr;
    FindBestTarget(PerceivedActors, SelfCharacter, BestTarget);
        
    AICon->SetTargetActor(Cast<ABaseCharacter>(BestTarget));
}

void UBTService_UpdateTarget_DM::FindBestTarget(TArray<AActor*> PerceivedActors, ABaseCharacter* SelfPawn, ABaseCharacter*& OutBestTarget) {
    float BestDistSq = TNumericLimits<float>::Max();

    const FVector SelfLoc = SelfPawn->GetActorLocation();
    ETeamId MyTeamId = SelfPawn->GetTeamId();

    for (AActor* Actor : PerceivedActors)
    {
        if (!Actor || Actor == SelfPawn) continue;

        ABaseCharacter* TargetChar = Cast<ABaseCharacter>(Actor);

        if (!TargetChar) continue;

        if (!TargetChar->IsAlive()) continue;

        // Nearest
        const float DistSq = FVector::DistSquared(SelfLoc, Actor->GetActorLocation());
        if (DistSq < BestDistSq)
        {
            BestDistSq = DistSq;
            OutBestTarget = TargetChar;
        }
    }
}