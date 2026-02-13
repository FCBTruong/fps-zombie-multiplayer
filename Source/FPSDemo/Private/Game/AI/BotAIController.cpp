#include "Game/AI/BotAIController.h"
#include "Game/Characters/BaseCharacter.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Game/Framework/MyPlayerState.h"
#include "Shared/Types/ItemId.h"
#include "Game/Characters/Components/WeaponFireComponent.h"
#include "Game/Characters/Components/SpikeComponent.h"
#include "Shared/Data/GlobalDataAsset.h"
#include "Game/GameManager.h"
#include "Game/Characters/Components/EquipComponent.h"

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

	bCanShoot = true;
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
    BindPawn(InPawn);

    // Get game mode and set initial match mode
    AShooterGameState* GS = GetWorld() ? GetWorld()->GetGameState<AShooterGameState>() : nullptr;
    if (GS)
    {
        UE_LOG(LogTemp, Warning, TEXT("BotAIController: Initial MatchMode=%d"), (uint8)GS->GetMatchMode());
        SetMatchMode(GS->GetMatchMode());

        GS->OnUpdateMatchState.AddUObject(
            this,
            &ABotAIController::SetMatchState
		);
		SetMatchState(GS->GetMatchState());
    }
}

void ABotAIController::OnUnPossess() {
    UnbindPawn();
    Super::OnUnPossess();
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
}

void ABotAIController::ResetAIState()
{
	SetTargetActor(nullptr);
    SetHasLineSight(false);
	SetPlantLocation(FVector::ZeroVector);
	SetCanShoot(true);

    // Clear AI focus
    ClearFocus(EAIFocusPriority::Gameplay);

    // Clear perception knowledge
    if (PerceptionComp)
    {
        PerceptionComp->ForgetAll();
    }
}

void ABotAIController::StartPlantingSpike() {
    ABaseCharacter* MyChar = GetBotChar();

    if (!MyChar) return;
    UE_LOG(LogTemp, Warning, TEXT("BotAIController: StartPlantingSpike called"));

    if (UEquipComponent* EC = MyChar->GetEquipComponent())
    {
        if (EC->GetActiveItemId() != EItemId::SPIKE)
        {
            EC->RequestSelectActiveItem(EItemId::SPIKE);
        }
        MyChar->RequestPrimaryActionPressed();
    }
}

void ABotAIController::StartDefusingSpike() {
    ABaseCharacter* MyChar = GetBotChar();
    if (!MyChar) return;

    if (USpikeComponent* WC = MyChar->GetSpikeComponent())
    {
        WC->RequestStartDefuseSpike();
    }
}

void ABotAIController::RequestFireOnce()
{
    if (!bCanShoot) {
        return;
	}

    ABaseCharacter* MyChar = GetBotChar();
    if (!MyChar) return;
	UWeaponFireComponent* WFC = MyChar->GetWeaponFireComponent();
	auto CanFireState = WFC->CanFireNow();
    if (CanFireState != EFireEnableReason::OK) {
        return;
    }
    MyChar->RequestPrimaryActionPressed();
    MyChar->RequestPrimaryActionReleased();
}

void ABotAIController::OnAmmoChanged(int32 Clip, int32 Reserve) // for current active weapon
{
	UE_LOG(LogTemp, Warning, TEXT("BotAIController::OnAmmoChanged: Clip=%d, Reserve=%d"), Clip, Reserve);
    if (Clip <= 0 && Reserve > 0)
    {
        SetCanShoot(false);
        GetWorldTimerManager().ClearTimer(ReloadDelayHandle);

        GetWorldTimerManager().SetTimer(
            ReloadDelayHandle,
            this,
            &ABotAIController::StartReload,
            0.5f,
            false
        );
    }
    else {
        GetWorldTimerManager().ClearTimer(ReloadDelayHandle);
    }
}

void ABotAIController::StartReloadDelayed()
{
    StartReload();
}

void ABotAIController::StartReload() {
    ABaseCharacter* MyChar = GetBotChar();
    if (!MyChar) {
        return;
    }
    MyChar->RequestReloadPressed();
}

void ABotAIController::SetTargetActor(ABaseCharacter* NewTarget)
{
    if (NewTarget) {
        LastTimeSeenTarget = GetWorld()->GetTimeSeconds();
    }
	TargetActor = NewTarget;
    UBlackboardComponent* BB = GetBlackboardComponent();
    if (BB)
    {
        BB->SetValueAsObject(BotBBKeys::TargetActor, NewTarget);
    }
}

ABaseCharacter* ABotAIController::GetTargetActor() const
{
    return TargetActor;
}

void ABotAIController::SetMatchMode(EMatchMode NewMode)
{
    CurrentMatchMode = NewMode;
    UBlackboardComponent* BB = GetBlackboardComponent();
    if (BB)
    {
        Blackboard->SetValueAsEnum(
            BotBBKeys::MatchMode,
            static_cast<uint8>(NewMode)
        );
    }
}

void ABotAIController::SetSpikeRole(EBotRole NewRole)
{
	SpikeRole = NewRole;
    UBlackboardComponent* BB = GetBlackboardComponent();
    if (BB)
    {
        BB->SetValueAsEnum(
            BotBBKeys::SpikeRole,
            static_cast<uint8>(NewRole)
        );
    }
}

void ABotAIController::SetSpikeActor(AActor* NewSpikeActor)
{
	SpikeActor = NewSpikeActor;
    UBlackboardComponent* BB = GetBlackboardComponent();
    if (BB)
    {
        BB->SetValueAsObject(
            BotBBKeys::SpikeActor,
            NewSpikeActor
        );
    }
}

void ABotAIController::SetIsAttacker(bool bAttacker)
{
	bIsAttacker = bAttacker;
    UBlackboardComponent* BB = GetBlackboardComponent();
    if (BB)
    {
        BB->SetValueAsBool(
            BotBBKeys::IsAttacker,
            bAttacker
        );
    }
}

void ABotAIController::SetPlantLocation(const FVector& NewLocation)
{
	PlantLocation = NewLocation;
    UBlackboardComponent* BB = GetBlackboardComponent();
    if (BB)
    {
        BB->SetValueAsVector(
            BotBBKeys::PlantLocation,
            NewLocation
        );
    }
}

void ABotAIController::SetCharacterRole(ECharacterRole NewRole)
{
	CharacterRole = NewRole;
    UBlackboardComponent* BB = GetBlackboardComponent();
    if (BB)
    {
        BB->SetValueAsEnum(
            BotBBKeys::CharacterRole,
            static_cast<uint8>(NewRole)
        );
    }
}

void ABotAIController::SetScoutLocation(const FVector& NewLocation)
{
    ScoutLocation = NewLocation;
    UBlackboardComponent* BB = GetBlackboardComponent();
    if (BB)
    {
        BB->SetValueAsVector(
            BotBBKeys::ScoutLocation,
            NewLocation
        );
    }
}

void ABotAIController::SetHasLineSight(bool bLineSight)
{
    bHasLineSight = bLineSight;
    if (UBlackboardComponent* BB = GetBlackboardComponent())
    {
        BB->SetValueAsBool(BotBBKeys::HasLineSight, bLineSight);
    }
}

void ABotAIController::SetHoldLocation(const FVector& NewLocation)
{
    HoldLocation = NewLocation;
    UBlackboardComponent* BB = GetBlackboardComponent();
    if (BB)
    {
        BB->SetValueAsVector(
            BotBBKeys::HoldLocation,
            NewLocation
        );
    }
}

void ABotAIController::BindPawn(APawn* InPawn)
{
    CachedChar = Cast<ABaseCharacter>(InPawn);
    if (!CachedChar.IsValid()) return;

    if (UEquipComponent* EquipComp = CachedChar->GetEquipComponent())
    {
        EquipComp->OnAmmoChanged.AddUObject(this, &ABotAIController::OnAmmoChanged);
    }
    if (UWeaponFireComponent* WFC = CachedChar->GetWeaponFireComponent())
    {
        WFC->OnFinishedReload.AddUObject(this, &ABotAIController::HandleFinishedReload);
	}
}

void ABotAIController::UnbindPawn()
{
    if (!CachedChar.IsValid()) return;

    if (UEquipComponent* EquipComp = CachedChar->GetEquipComponent())
    {
        EquipComp->OnAmmoChanged.RemoveAll(this);
    }

    CachedChar.Reset();
}

void ABotAIController::UpdateControlRotation(float DeltaTime, bool bUpdatePawn)
{
    APawn* const MyPawn = GetPawn();
    if (MyPawn)
    {
        FRotator NewControlRotation = GetControlRotation();

        // Look toward focus
        const FVector FocalPoint = GetFocalPoint();
        if (FAISystem::IsValidLocation(FocalPoint))
        {
            NewControlRotation = (FocalPoint - MyPawn->GetPawnViewLocation()).Rotation();
        }
        else if (bSetControlRotationFromPawnOrientation)
        {
            NewControlRotation = MyPawn->GetActorRotation();
        }

        SetControlRotation(NewControlRotation);

        if (bUpdatePawn)
        {
            const FRotator CurrentPawnRotation = MyPawn->GetActorRotation();

            if (CurrentPawnRotation.Equals(NewControlRotation, 1e-3f) == false)
            {
                MyPawn->FaceRotation(NewControlRotation, DeltaTime);
            }
        }
    }
}

void ABotAIController::SetCanShoot(bool bInCanShoot)
{
	bCanShoot = bInCanShoot;
    UBlackboardComponent* BB = GetBlackboardComponent();
    if (BB)
    {
        BB->SetValueAsBool(
            BotBBKeys::CanShoot,
            bCanShoot
        );
    }
}

void ABotAIController::HandleFinishedReload()
{
    GetWorldTimerManager().ClearTimer(FinishedReloadDelayHandle);

    GetWorldTimerManager().SetTimer(
        FinishedReloadDelayHandle,
        this,
        &ABotAIController::SetCanShootTrueDelayed,
        0.5f,
        false
    );
}

void ABotAIController::SetCanShootTrueDelayed()
{
    SetCanShoot(true);
}

void ABotAIController::SetMatchState(EMyMatchState NewState)
{
    CurrentMatchState = NewState;
    UBlackboardComponent* BB = GetBlackboardComponent();
    if (BB)
    {
        Blackboard->SetValueAsEnum(
            BotBBKeys::MatchState,
            static_cast<uint8>(NewState)
        );
    }
}