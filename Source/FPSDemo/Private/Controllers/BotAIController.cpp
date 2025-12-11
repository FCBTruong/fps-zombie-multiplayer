#include "Controllers/BotAIController.h"
#include "Characters/BaseCharacter.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Controllers/MyPlayerState.h"

ABotAIController::ABotAIController()
{
    PrimaryActorTick.bCanEverTick = true; 

    PerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>("Perception");
    SetPerceptionComponent(*PerceptionComp);
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>("SightConfig");

    SightConfig->SightRadius = 5000.f;
    SightConfig->LoseSightRadius = 5500.f;
    SightConfig->PeripheralVisionAngleDegrees = 180.f;

    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;

    PerceptionComp->ConfigureSense(*SightConfig);
    PerceptionComp->SetDominantSense(UAISense_Sight::StaticClass());

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
    
}


void ABotAIController::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (!GetPawn()) return;

    FVector Loc = GetPawn()->GetActorLocation();
    FRotator Rot = GetPawn()->GetActorRotation();

    // Sight radius
    DrawDebugCircle(
        GetWorld(),
        Loc,
        SightConfig->SightRadius,
        32,
        FColor::Green,
        false,
        -1,
        0,
        2,
        FVector(1, 0, 0),
        FVector(0, 1, 0),
        false
    );

    // FOV direction lines
    float HalfFOV = SightConfig->PeripheralVisionAngleDegrees;
    FVector Fwd = Rot.Vector();

    FVector LeftDir = Fwd.RotateAngleAxis(-HalfFOV, FVector::UpVector);
    FVector RightDir = Fwd.RotateAngleAxis(+HalfFOV, FVector::UpVector);

    DrawDebugLine(GetWorld(), Loc, Loc + LeftDir * SightConfig->SightRadius, FColor::Blue, false, -1, 0, 2);
    DrawDebugLine(GetWorld(), Loc, Loc + RightDir * SightConfig->SightRadius, FColor::Blue, false, -1, 0, 2);
}


void ABotAIController::ResetAIState()
{
    UBlackboardComponent* BB = GetBlackboardComponent();
    if (!BB) return;

    // Clear target data
    BB->ClearValue("TargetActor");
    BB->ClearValue("HasLineOfSight");
    BB->ClearValue("TargetLocation");

    // Clear AI focus
    ClearFocus(EAIFocusPriority::Gameplay);

    // Clear perception knowledge
    if (PerceptionComp)
    {
        PerceptionComp->ForgetAll();
    }
}
