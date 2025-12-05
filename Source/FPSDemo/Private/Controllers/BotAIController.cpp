#include "Controllers/BotAIController.h"
#include "Characters/BaseCharacter.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Controllers/MyPlayerState.h"

ABotAIController::ABotAIController()
{
    PrimaryActorTick.bCanEverTick = false; 

    PerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>("Perception");
    SetPerceptionComponent(*PerceptionComp);
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>("SightConfig");

    SightConfig->SightRadius = 3000.f;
    SightConfig->LoseSightRadius = 3500.f;
    SightConfig->PeripheralVisionAngleDegrees = 90.f;

    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;

    PerceptionComp->ConfigureSense(*SightConfig);
    PerceptionComp->SetDominantSense(UAISenseConfig_Sight::StaticClass());

    PerceptionComp->OnPerceptionUpdated.AddDynamic(this, &ABotAIController::OnPerceptionUpdated);
}

void ABotAIController::BeginPlay()
{
    Super::BeginPlay();
}
void ABotAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    ABaseCharacter* BC = Cast<ABaseCharacter>(InPawn);
    if (BC && BC->BehaviorTree)
    {
        UE_LOG(LogTemp, Log, TEXT("BotAIController: Running Behavior Tree"));
        RunBehaviorTree(BC->BehaviorTree);
    }
}

void ABotAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    AActor* NewTarget = nullptr;
    
    AMyPlayerState* MyPS = GetPlayerState<AMyPlayerState>();
    if (!MyPS)
    {
        UE_LOG(LogTemp, Warning, TEXT("BotAIController: My PlayerState is null"));
        return;
	}
    FName MyTeamId = MyPS->GetTeamID();

    for (AActor* Actor : UpdatedActors)
    {
        // check is enemy or ally
        ABaseCharacter* BC = Cast<ABaseCharacter>(Actor);
        if (!BC)
            continue;

		AMyPlayerState* BC_PS = BC->GetPlayerState<AMyPlayerState>();
        if (!BC_PS)
        {
            continue;
        }
        if (BC_PS->GetTeamID() == MyTeamId)
        {
            continue; // same team, skip
		}

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
	UE_LOG(LogTemp, Log, TEXT("BotAIController: New CurrentTarget is %s"), 
		CurrentTarget ? *CurrentTarget->GetName() : TEXT("None"));

    if (Blackboard)
    {
        UE_LOG(LogTemp, Log, TEXT("BotAIController: Updating TargetActor in Blackboard to %s"), 
			CurrentTarget ? *CurrentTarget->GetName() : TEXT("None"));
        Blackboard->SetValueAsObject("TargetActor", CurrentTarget);
    }
}
