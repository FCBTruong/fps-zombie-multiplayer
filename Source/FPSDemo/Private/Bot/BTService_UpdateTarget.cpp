#include "Bot/BTService_UpdateTarget.h"
#include "Controllers/BotAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Characters/BaseCharacter.h"
#include "Perception/AISense_Sight.h"
#include "GameFramework/PlayerState.h"
#include "Controllers/MyPlayerState.h"

UBTService_UpdateTarget::UBTService_UpdateTarget()
{
	UE_LOG(LogTemp, Log, TEXT("BTService_UpdateTarget: Constructor called"));
    bNotifyTick = true;
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
    APawn* Pawn = AICon->GetPawn();
    if (!Pawn || !Perception || !BB) return;

    // Get my team ID
    AMyPlayerState* MyPS = Pawn->GetPlayerState<AMyPlayerState>();
    if (!MyPS) return;

    FName MyTeamID = MyPS->GetTeamID();

    // Collect all perceived actors by sight
    TArray<AActor*> PerceivedActors;
    Perception->GetCurrentlyPerceivedActors(UAISense_Sight::StaticClass(), PerceivedActors);

    AActor* BestTarget = nullptr;

    // Loop through perceived actors
    for (AActor* Actor : PerceivedActors)
    {
        ABaseCharacter* BC = Cast<ABaseCharacter>(Actor);
        if (!BC) continue;

        AMyPlayerState* TargetPS = BC->GetPlayerState<AMyPlayerState>();
        if (!TargetPS) continue;

        // Skip dead players
        if (!TargetPS->IsAlive()) continue;

        // Skip same team
        if (TargetPS->GetTeamID() == MyTeamID) continue;

        // This is an enemy we can see
        BestTarget = Actor;
        break;
    }

    // Update blackboard
    BB->SetValueAsObject("Obj_TargetActor", BestTarget);

    if (BestTarget)
    {
        FVector MyLoc = Pawn->GetActorLocation();
        FVector TargetLoc = BestTarget->GetActorLocation();

        // LOS trace
        FHitResult Hit;
        FCollisionQueryParams Params;
        Params.AddIgnoredActor(Pawn);

        bool bHit = Pawn->GetWorld()->LineTraceSingleByChannel(
            Hit, MyLoc, TargetLoc, ECC_Visibility, Params
        );

        bool bHasLOS = (!bHit || Hit.GetActor() == BestTarget);

        BB->SetValueAsBool("B_HasLineSight", bHasLOS);
        BB->SetValueAsVector("Vec_TargetLocation", TargetLoc);
    }
    else
    {
        BB->SetValueAsBool("B_HasLineSight", false);
    }
    AICon->SetFocus(BestTarget);
}
