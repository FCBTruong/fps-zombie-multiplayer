#include "Controllers/BotAIController.h"
#include "Characters/BaseCharacter.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Controllers/MyPlayerState.h"
#include "Items/ItemIds.h"
#include "Components/WeaponComponent.h"

ABotAIController::ABotAIController()
{
    PrimaryActorTick.bCanEverTick = true; 

    PerceptionComp = CreateDefaultSubobject<UAIPerceptionComponent>("Perception");
    SetPerceptionComponent(*PerceptionComp);
    SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>("SightConfig");

    SightConfig->SightRadius = 5000.f;
    SightConfig->LoseSightRadius = 5500.f;
    SightConfig->PeripheralVisionAngleDegrees = 80.f;

    SightConfig->DetectionByAffiliation.bDetectEnemies = true;
    SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
    SightConfig->DetectionByAffiliation.bDetectNeutrals = true;

    PerceptionComp->ConfigureSense(*SightConfig);
    PerceptionComp->SetDominantSense(UAISense_Sight::StaticClass());

    DamageConfig = CreateDefaultSubobject<UAISenseConfig_Damage>("DamageConfig");
    PerceptionComp->ConfigureSense(*DamageConfig);

    PerceptionComp->OnPerceptionUpdated.AddDynamic(this, &ABotAIController::OnPerceptionUpdated);
    PerceptionComp->OnTargetPerceptionUpdated.AddDynamic(
        this, &ABotAIController::OnTargetPerceptionUpdated);


}

void ABotAIController::BeginPlay()
{
    Super::BeginPlay();
}
void ABotAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    ABaseCharacter* BC = Cast<ABaseCharacter>(InPawn);
    if (BC && BC->GetBehaviorTree())
    {
        UE_LOG(LogTemp, Log, TEXT("BotAIController: Running Behavior Tree"));
        RunBehaviorTree(BC->GetBehaviorTree());
    }
}

void ABotAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
    
}

void ABotAIController::OnTargetPerceptionUpdated(
    AActor* Actor,
    FAIStimulus Stimulus)
{
	
}


void ABotAIController::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (!GetPawn()) return;

    FVector Loc = GetPawn()->GetActorLocation();
    FRotator Rot = GetPawn()->GetActorRotation();

    // Sight radius
   /* DrawDebugCircle(
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
    );*/

    // FOV direction lines
    float HalfFOV = SightConfig->PeripheralVisionAngleDegrees;
    FVector Fwd = Rot.Vector();

    FVector LeftDir = Fwd.RotateAngleAxis(-HalfFOV, FVector::UpVector);
    FVector RightDir = Fwd.RotateAngleAxis(+HalfFOV, FVector::UpVector);

   /* DrawDebugLine(GetWorld(), Loc, Loc + LeftDir * SightConfig->SightRadius, FColor::Blue, false, -1, 0, 2);
    DrawDebugLine(GetWorld(), Loc, Loc + RightDir * SightConfig->SightRadius, FColor::Blue, false, -1, 0, 2);*/
}


void ABotAIController::ResetAIState()
{
    UBlackboardComponent* BB = GetBlackboardComponent();
    if (!BB) return;

    // Clear target data
    BB->ClearValue("Obj_TargetActor");
    BB->ClearValue("B_HasLineSight");
    BB->ClearValue("Vec_TargetLocation");
    BB->ClearValue("B_IsInBombArea");
	BB->ClearValue("Vec_SpikeLocation");
    BB->ClearValue("Vec_PlantLocation");
	BB->ClearValue("Name_BombSite");
    BB->ClearValue("Vec_HoldLocation");

    // Clear AI focus
    ClearFocus(EAIFocusPriority::Gameplay);

    // Clear perception knowledge
    if (PerceptionComp)
    {
        PerceptionComp->ForgetAll();
    }
}


void ABotAIController::StartPlantingSpike() {
    APawn* MyPawn = GetPawn();
    if (!MyPawn) return;
    if (ABaseCharacter* MyChar = Cast<ABaseCharacter>(MyPawn))
    {
        if (UWeaponComponent* WC = MyChar->FindComponentByClass<UWeaponComponent>())
        {
            if (WC->GetCurrentWeaponType() == EWeaponTypes::Spike) {
                WC->OnInput_StartPlantSpike();
            }
            else {
				WC->EquipWeapon(EItemId::SPIKE);
				WC->OnInput_StartPlantSpike();
            }
        }
    }
}

void ABotAIController::StartDefusingSpike() {
    APawn* MyPawn = GetPawn();
    if (!MyPawn) return;
    if (ABaseCharacter* MyChar = Cast<ABaseCharacter>(MyPawn))
    {
        if (UWeaponComponent* WC = MyChar->FindComponentByClass<UWeaponComponent>())
        {     
           WC->OnInput_StartDefuseSpike();
        }
    }
}