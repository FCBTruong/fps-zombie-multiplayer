#include "Game/AI/BTService_UpdateTarget.h"
#include "Game/AI/BotAIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Game/Characters/BaseCharacter.h"
#include "Perception/AISense_Sight.h"
#include "GameFramework/PlayerState.h"
#include "Game/Framework/MyPlayerState.h"

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

    UAIPerceptionComponent* Perception = AICon->GetAIPerceptionComponent();
    APawn* Pawn = AICon->GetPawn();
    if (!Pawn || !Perception) return;
	ABaseCharacter* MyBotCharacter = Cast<ABaseCharacter>(Pawn);
	if (!MyBotCharacter) return;

    // Get my team ID
    AMyPlayerState* MyPS = Pawn->GetPlayerState<AMyPlayerState>();
    if (!MyPS) return;

    ETeamId MyTeamId = MyPS->GetTeamId();

    // Collect all perceived actors by sight
    TArray<AActor*> PerceivedActors;
    Perception->GetCurrentlyPerceivedActors(UAISense_Sight::StaticClass(), PerceivedActors);
	UE_LOG(LogTemp, Log, TEXT("BTService_UpdateTarget: Perceived %d actors"), PerceivedActors.Num());

    ABaseCharacter* BestTarget = nullptr;

    // Loop through perceived actors
    for (AActor* Actor : PerceivedActors)
    {
        ABaseCharacter* BC = Cast<ABaseCharacter>(Actor);
        if (!BC) continue;

        AMyPlayerState* TargetPS = BC->GetPlayerState<AMyPlayerState>();
        if (!TargetPS) continue;

        // Skip dead players
        if (BC->IsDead()) continue;

        // Skip same team
        if (TargetPS->GetTeamId() == MyTeamId) continue;

		if (!MyBotCharacter->CanSeeThisActor(BC)) continue;

        // This is an enemy we can see
        BestTarget = BC;
        break;
    }

	AICon->SetTargetActor(BestTarget);

    if (BestTarget)
    {
        FVector AimLoc = BestTarget->GetActorLocation();
        
        AimLoc = BestTarget->GetAimPoint(EAimPointPolicy::HeadOrBody, 0.2f); // 50% head

		const bool bHasLOS = MyBotCharacter->CanSeeThisActor(BestTarget);
        // Focus the point
        AICon->SetFocalPoint(AimLoc, EAIFocusPriority::Gameplay);
    }
    else
    {
        AICon->ClearFocus(EAIFocusPriority::Gameplay);
    }
}
