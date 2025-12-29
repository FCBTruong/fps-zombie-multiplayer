#include "Controllers/BotAIController.h"
#include "Characters/BaseCharacter.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Controllers/MyPlayerState.h"
#include "Items/ItemIds.h"
#include "Components/WeaponFireComponent.h"
#include "Components/SpikeComponent.h"
#include "Game/GlobalDataAsset.h"
#include "Game/GameManager.h"
#include "Components/EquipComponent.h"

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

    UGameManager* GMR = Cast<UGameManager>(GetWorld()->GetGameInstance());
    if (GMR && GMR->GlobalData)
    {
        if (GMR->GlobalData->BotBehaviorTree) {
            RunBehaviorTree(GMR->GlobalData->BotBehaviorTree);
        }
    }
	ABaseCharacter* MyChar = Cast<ABaseCharacter>(InPawn);
    if (MyChar) {
        UEquipComponent* EquipComp = MyChar->GetEquipComponent();
        EquipComp->OnAmmoChanged.AddUObject(this, &ABotAIController::OnAmmoChanged);
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
	UE_LOG(LogTemp, Warning, TEXT("BotAIController: StartPlantingSpike called"));
    if (ABaseCharacter* MyChar = Cast<ABaseCharacter>(MyPawn))
    {
        if (UEquipComponent* EC = MyChar->GetEquipComponent())
        {
            if (EC->GetActiveItemId() != EItemId::SPIKE)
            {
                EC->RequestSelectActiveItem(EItemId::SPIKE);
			}
            if (USpikeComponent* SC = MyChar->GetSpikeComponent())
            {
                SC->RequestPlantSpike();
            }
        }
    }
}

void ABotAIController::StartDefusingSpike() {
    APawn* MyPawn = GetPawn();
    if (!MyPawn) return;
    if (ABaseCharacter* MyChar = Cast<ABaseCharacter>(MyPawn))
    {
        if (USpikeComponent* WC = MyChar->GetSpikeComponent())
        {     
			WC->RequestStartDefuseSpike();
        }
    }
}

void ABotAIController::RequestFireOnce()
{
    APawn* MyPawn = GetPawn();
    if (!MyPawn) return;
    if (ABaseCharacter* MyChar = Cast<ABaseCharacter>(MyPawn))
    {
        if (UWeaponFireComponent* WC = MyChar->GetWeaponFireComponent())
        {
            WC->RequestFireOnce();
        }
    }
}

void ABotAIController::OnAmmoChanged(int32 Clip, int32 Reserve) // for current active weapon
{
	UE_LOG(LogTemp, Warning, TEXT("BotAIController::OnAmmoChanged: Clip=%d, Reserve=%d"), Clip, Reserve);
    if (Clip <= 0 && Reserve > 0)
    {
        APawn* MyPawn = GetPawn();
        if (!MyPawn) return;
        if (ABaseCharacter* MyChar = Cast<ABaseCharacter>(MyPawn))
        {
            if (UWeaponFireComponent* WFC = MyChar->GetWeaponFireComponent()) {
                WFC->RequestReload();
            }
        }
	}
}