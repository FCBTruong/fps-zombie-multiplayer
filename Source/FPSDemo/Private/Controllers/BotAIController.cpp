#include "Controllers/BotAIController.h"
#include "Characters/BaseCharacter.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"

ABotAIController::ABotAIController()
{
    PrimaryActorTick.bCanEverTick = false; 

    PerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>("Perception");
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>("SightConfig");

    SightConfig->SightRadius = 3000.f;
    SightConfig->LoseSightRadius = 3500.f;
    SightConfig->PeripheralVisionAngleDegrees = 90.f;

    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = false;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = false;

    PerceptionComp->ConfigureSense(*SightConfig);
    PerceptionComp->SetDominantSense(UAISenseConfig_Sight::StaticClass());

    PerceptionComp->OnPerceptionUpdated.AddDynamic(this, &ABotAIController::OnPerceptionUpdated);
}

void ABotAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    ABaseCharacter* BC = Cast<ABaseCharacter>(InPawn);
    if (BC && BC->BehaviorTree)
    {
        RunBehaviorTree(BC->BehaviorTree);
    }
}

void ABotAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    AActor* NewTarget = nullptr;

    for (AActor* Actor : UpdatedActors)
    {
        FActorPerceptionBlueprintInfo Info;
        PerceptionComp->GetActorsPerception(Actor, Info);

        if (Info.LastSensedStimuli.Num() > 0 &&
            Info.LastSensedStimuli[0].WasSuccessfullySensed())
        {
            NewTarget = Actor;
            break;
        }
    }

    CurrentTarget = NewTarget;

    if (Blackboard)
    {
        Blackboard->SetValueAsObject("TargetActor", CurrentTarget);
    }
}
