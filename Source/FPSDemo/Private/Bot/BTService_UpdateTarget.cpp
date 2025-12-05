// Fill out your copyright notice in the Description page of Project Settings.


#include "Bot/BTService_UpdateTarget.h"
#include "Controllers/BotAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Characters/BaseCharacter.h"
#include "Perception/AISense_Sight.h"

UBTService_UpdateTarget::UBTService_UpdateTarget()
{
    Interval = 0.1f;
    RandomDeviation = 0.f;
}

void UBTService_UpdateTarget::TickNode(
    UBehaviorTreeComponent& OwnerComp,
    uint8* NodeMemory,
    float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    ABotAIController* AICon = Cast<ABotAIController>(OwnerComp.GetAIOwner());
    if (!AICon) return;

    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    UAIPerceptionComponent* Perception = AICon->GetAIPerceptionComponent();

    AActor* BestTarget = nullptr;

    TArray<AActor*> PerceivedActors;
    Perception->GetCurrentlyPerceivedActors(UAISense_Sight::StaticClass(), PerceivedActors);

    if (PerceivedActors.Num() > 0)
    {
        BestTarget = PerceivedActors[0];
    }

    BB->SetValueAsObject("TargetActor", BestTarget);

    if (BestTarget)
    {
        const FVector MyLoc = AICon->GetPawn()->GetActorLocation();
        const FVector TargetLoc = BestTarget->GetActorLocation();

        FHitResult Hit;
        FCollisionQueryParams Params;
        Params.AddIgnoredActor(AICon->GetPawn());

        bool bHit = AICon->GetWorld()->LineTraceSingleByChannel(
            Hit, MyLoc, TargetLoc, ECC_Visibility, Params
        );

        bool HasLOS = (!bHit || Hit.GetActor() == BestTarget);

        BB->SetValueAsBool("HasLineOfSight", HasLOS);
        BB->SetValueAsVector("TargetLocation", TargetLoc);
    }
    else
    {
        BB->SetValueAsBool("HasLineOfSight", false);
    }
}