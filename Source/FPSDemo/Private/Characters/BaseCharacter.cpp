#include "Characters/BaseCharacter.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Weapons/WeaponKnifeBasic.h"
#include "Kismet/GameplayStatics.h"
#include "Structs/InventoryItem.h"
#include "Game/ShooterGameMode.h"
#include "Projectiles/ThrownProjectile.h"
#include "Materials/MaterialParameterCollectionInstance.h"
#include "Controllers/MyPlayerController.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Damage/MyDamageType.h"
#include "Engine/DamageEvents.h"
#include "Damage/MyPointDamageEvent.h"
#include "Controllers/MyPlayerState.h"
#include "Game/ShooterGameState.h"
#include "Perception/AISense_Damage.h"
#include "Controllers/BotAIController.h"
#include "Weapons/WeaponState.h"
#include "GameFramework/Character.h"
#include "Weapons/WeaponTypes.h"
#include "Weapons/WeaponBase.h"
#include "Net/UnrealNetwork.h"
#include "Camera/CameraComponent.h"
#include "Components/TimelineComponent.h"
#include "Components/InteractComponent.h"
#include "Components/HealthComponent.h"     
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Components/PostProcessComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/Material.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Components/AudioComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/PickupComponent.h"
#include "Components/InventoryComponent.h"
#include "Components/WeaponComponent.h"
#include "Components/AnimationComponent.h"
#include "Components/CharAudioComponent.h"
#include "Components/CharCameraComponent.h"

// Sets default values
ABaseCharacter::ABaseCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    FpsPivot = CreateDefaultSubobject<USceneComponent>(TEXT("FpsPivot"));
    FpsPivot->SetupAttachment(GetRootComponent());

    CameraFps = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraFps"));
    CameraFps->SetupAttachment(FpsPivot);
    CameraFps->bUsePawnControlRotation = false;

    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBloom"));
    CameraBoom->SetupAttachment(RootComponent);

    CameraTps = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraTps"));
    CameraTps->SetupAttachment(CameraBoom);
    CameraTps->bUsePawnControlRotation = false;

    MeshFps = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshFps"));
    MeshFps->SetupAttachment(FpsPivot);

    ViewmodelCap = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("ViewmodelCap"));
    ViewmodelCap->SetupAttachment(CameraFps);
    ViewmodelCap->bCaptureEveryFrame = true;
    ViewmodelCap->bCaptureOnMovement = true;
    ViewmodelCap->PrimitiveRenderMode =
        ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;

    ThrowSpline = CreateDefaultSubobject<USplineComponent>(TEXT("SplineThrow"));
    ThrowSpline->SetupAttachment(RootComponent);

	ThrowableLocation = CreateDefaultSubobject<USceneComponent>(TEXT("ThrowableLocation"));
	ThrowableLocation->SetupAttachment(RootComponent);

	UE_LOG(LogTemp, Warning, TEXT("ABaseCharacter constructor called"));

    StimuliSource = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("StimuliSource"));

    PickupComponent = CreateDefaultSubobject<UPickupComponent>(TEXT("PickupComponent"));
    InventoryComp = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));
    WeaponComp = CreateDefaultSubobject<UWeaponComponent>(TEXT("WeaponComponent"));
    InteractComp = CreateDefaultSubobject<UInteractComponent>(TEXT("InteractComponent"));
    HealthComp = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
    AnimationComp = CreateDefaultSubobject<UAnimationComponent>(TEXT("AnimationComponent"));
    AudioComp = CreateDefaultSubobject<UCharAudioComponent>(TEXT("AudioComponent"));
    CameraComp = CreateDefaultSubobject<UCharCameraComponent>(TEXT("CharCameraComponent"));

    CameraComp->Initialize(
        CameraFps,
        CameraTps,
        CameraBoom,
        ViewmodelCap,
        MeshFps,
        GetMesh()
    );

    if (WeaponComp) {
		UE_LOG(LogTemp, Warning, TEXT("DEBUGGG:: WeaponComp is valid in ABaseCharacter constructor"));
    }

    if (GetMesh()) {
        GetMesh()->SetRelativeLocation(FVector(0.f, 0.f, -88.f));
        GetMesh()->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));

        static ConstructorHelpers::FClassFinder<UAnimInstance> AnimBPClass(
            TEXT("/Game/Main/ABP_Character_TPS.ABP_Character_TPS_C"));

        if (AnimBPClass.Succeeded())
        {
            GetMesh()->SetAnimInstanceClass(AnimBPClass.Class);
        }
    }
    if (FpsPivot) {
        // set same eye position
        BaseEyeHeight = 70;
		FpsPivot->SetRelativeLocation(FVector(0.f, 0.f, BaseEyeHeight));
    }
    if (MeshFps)
    {
        MeshFps->SetRelativeLocation(FVector(0.f, 0.f, -170.f));
        MeshFps->SetRelativeRotation(FRotator(0.f, -90.f, 0.f));

        static ConstructorHelpers::FObjectFinder<USkeletalMesh> MeshFinder(
            TEXT("/Game/Main/Core/Mannequins/Meshes/SK_FP_Manny_Simple.SK_FP_Manny_Simple")
        );

        if (MeshFinder.Succeeded())
        {
            MeshFps->SetSkeletalMesh(MeshFinder.Object);
        }
        MeshFps->SetAnimationMode(EAnimationMode::AnimationBlueprint);

        static ConstructorHelpers::FClassFinder<UAnimInstance> FpsAnimBPClass(
            TEXT("/Game/Main/ABP_Character.ABP_Character_C")
        );

        if (FpsAnimBPClass.Succeeded())
        {
			UE_LOG(LogTemp, Warning, TEXT("FpsAnimBPClass succeeded"));
            MeshFps->SetAnimInstanceClass(FpsAnimBPClass.Class);
        }
        else {
			UE_LOG(LogTemp, Warning, TEXT("FpsAnimBPClass failed"));
        }
    }
}

// Called when the game starts or when spawned
void ABaseCharacter::BeginPlay()
{
    if (!WeaponComp) {
        UE_LOG(LogTemp, Warning, TEXT("DEBUGGG:: WeaponComp is not valid at beginplay"));
    }
    Super::BeginPlay();

    if (UGameManager* GM = UGameManager::Get(GetWorld()))
    {
        GM->RegisterPlayer(this);
    }

    BasePivotFpsZ = FpsPivot->GetRelativeLocation().Z;

    if (CrouchCurve)
    {
        FOnTimelineFloat Update;
        Update.BindUFunction(this, FName("HandleCrouchProgress"));
        CrouchTimeline.AddInterpFloat(CrouchCurve, Update);

        CrouchTimeline.SetLooping(false);
    }

    if (MeshFps) {
        if (UAnimInstance* FPSAnim = MeshFps->GetAnimInstance())
        {
            UE_LOG(LogTemp, Warning, TEXT("FPSAnim is valid in ABaseCharacter"));
            FPSAnim->OnPlayMontageNotifyBegin.AddDynamic(this, &ABaseCharacter::OnNotifyBegin);
        }
    }
    if (GetMesh()) {
        if (UAnimInstance* TPSAnim = GetMesh()->GetAnimInstance())
        {
            TPSAnim->OnPlayMontageNotifyBegin.AddDynamic(this, &ABaseCharacter::OnNotifyBegin);
        }
    }


    if (HealthComp)
    {
        HealthComp->OnDeath.AddUObject(this, &ABaseCharacter::HandleDeath);
    }

    if (StunCurve)
    {
        // Bind update function
        FOnTimelineFloat UpdateFunction;
        UpdateFunction.BindUFunction(this, FName("OnStunTimelineUpdate"));
        StunTimeline.AddInterpFloat(StunCurve, UpdateFunction);

        // Bind finished function (optional)
        FOnTimelineEvent FinishedFunction;
        FinishedFunction.BindUFunction(this, FName("OnStunTimelineFinished"));
        StunTimeline.SetTimelineFinishedFunc(FinishedFunction);

        StunTimeline.SetLooping(false);
        BaseStunDuration = StunTimeline.GetTimelineLength();
    }

   
    if (FlashCollection)
    {
        if (UMaterialParameterCollectionInstance* MPC =
            GetWorld()->GetParameterCollectionInstance(FlashCollection))
        {
            MPC->SetScalarParameterValue("Intensity", 0.0f);
        }
    }

    if (StimuliSource)
    {
        StimuliSource->RegisterForSense(UAISense_Sight::StaticClass());
        StimuliSource->RegisterWithPerceptionSystem();
	}

    CameraComp->OnViewModeChanged.AddUObject(
        WeaponComp,
        &UWeaponComponent::OnViewModeChanged
    );

    bHasBeginPlayRun = true;
}


void ABaseCharacter::OnRep_PlayerState() {
    Super::OnRep_PlayerState();
    // Set mesh based on team
    if (!bAppliedTeamMesh) {
        bAppliedTeamMesh = true;
        ApplyTeamMesh();
	}
}

void ABaseCharacter::ApplyTeamMesh()
{
	UE_LOG(LogTemp, Warning, TEXT("Setting mesh based on team"));
    UGameManager* GMR = GetWorld()->GetGameInstance()->GetSubsystem<UGameManager>();
    if (GMR && GMR->GlobalData)
    {
        // get player state
        AMyPlayerState* MyPS = Cast<AMyPlayerState>(GetPlayerState());
		AShooterGameState* GS = GetWorld()->GetGameState<AShooterGameState>();
		if (!GS) {
			UE_LOG(LogTemp, Warning, TEXT("GS is null in SetMeshBaseOnTeam"));
			return;
		}
        if (MyPS) {
            USkeletalMesh* NewMesh = nullptr;

            UE_LOG(LogTemp, Warning,
                TEXT("SetMeshBaseOnTeam: TeamID = %s"),
                *MyPS->GetTeamID().ToString());
            if (MyPS->GetTeamID() == GS->GetAttackerTeam()) {
                NewMesh = GMR->GlobalData->TerroristMesh;
            }
            else {
                NewMesh = GMR->GlobalData->CounterTerroristMesh;
            }
            if (NewMesh)
            {
				UE_LOG(LogTemp, Warning, TEXT("Setting new mesh for team"));
                GetMesh()->SetSkeletalMesh(NewMesh);
            }
        }
        else {
			UE_LOG(LogTemp, Warning, TEXT("MyPS is null in SetMeshBaseOnTeam"));
        }
    }
}

// Called every frame
void ABaseCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
   
    if (StunTimeline.IsPlaying()) {
        StunTimeline.TickTimeline(DeltaTime);
    }
    if (CrouchTimeline.IsPlaying()) {
        CrouchTimeline.TickTimeline(DeltaTime);
	}
    if (FpsPivot && Controller) {
        FRotator ControlRot = Controller->GetControlRotation();

        // FPS rule: pitch + yaw, no roll
        FRotator PivotRot(ControlRot.Pitch, 0.f, 0.f);

        FpsPivot->SetRelativeRotation(PivotRot);
    }
    // logic sound
    UpdateFootstepSound(DeltaTime);
}

void ABaseCharacter::UpdateFootstepSound(float DeltaTime) {
    const float Speed = GetVelocity().Size2D();
   
    if (!CanPlayFootstep()) {
        return;
    }

    const float CurrentTime = GetWorld()->GetTimeSeconds();

    float FootstepInterval = 0.35f;
    if (Speed > 500.f) {
        FootstepInterval = 0.25f;
    }
    else if (Speed > 450.f) {
        FootstepInterval = 0.3f;
	}
    if (CurrentTime - LastFootstepTime >= FootstepInterval)
    {
        LastFootstepTime = CurrentTime;
		PlayFootstepSound();
    }
}

bool ABaseCharacter::CanPlayFootstep() const
{
    return GetVelocity().Size2D() > ABaseCharacter::FOOTSTEP_SPEED_MIN &&
        !GetCharacterMovement()->IsFalling();
}

void ABaseCharacter::ApplyAimingVisuals()
{
    UE_LOG(LogTemp, Warning, TEXT("Updating Aiming State: %s"), bIsAiming ? TEXT("Aiming") : TEXT("Not Aiming"));
    // Get Player controller and show scope widget
    AMyPlayerController* PC = Cast<AMyPlayerController>(UGameplayStatics::GetPlayerController(this, 0));
    if (!PC || !PC->IsLocalController())
        return;

    if (PC->GetViewTarget() != this)
        return;

    if (!PC) {
        UE_LOG(LogTemp, Warning, TEXT("PlayerController is null in UpdateAimingState"));
        return;
    }
    if (!CameraComp) {
        UE_LOG(LogTemp, Warning, TEXT("CameraComp is null in UpdateAimingState"));
        return;
	}

    if (!WeaponComp) {
        UE_LOG(LogTemp, Warning, TEXT("WeaponComp is null in UpdateAimingState"));
        return;
	}
    if (bIsAiming)
    {
        if (WeaponComp->IsScopeEquipped()) {
            if (CameraComp) {
                CameraComp->SetTargetFOV(20.f);
            }
            AimSensitivity = 0.2f;
			OnAimingChanged.Broadcast(true);
        }
    }
    else
    {
        CameraComp->ResetFOV();
        AimSensitivity = 1.0f;
		OnAimingChanged.Broadcast(false);
    }
}

void ABaseCharacter::Jump()
{
    if (GetCharacterMovement()->IsCrouching()) {
        UnCrouch();
        return;
	}
    if (GetCharacterMovement()->IsFalling()) {
        return;
	}
    Super::Jump();
}


void ABaseCharacter::StopJumping()
{
    Super::StopJumping();
}

void ABaseCharacter::RequestCrouch()
{
    if (GetCharacterMovement()->IsFalling()) {
        return;
    }
    Crouch();
}

void ABaseCharacter::RequestUnCrouch()
{
    UnCrouch();
}

void ABaseCharacter::OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
    Super::OnStartCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);

    // Cancel the instant camera drop caused by capsule shrinking
    CurrentCrouchCompZ += HalfHeightAdjust;

    CrouchFromZ = CurrentCrouchCompZ;
    CrouchToZ = 0.f;

    // Snap to the compensated position (no pop), then smooth toward target
    FVector Loc = FpsPivot->GetRelativeLocation();
    Loc.Z = BasePivotFpsZ + CurrentCrouchCompZ;
    FpsPivot->SetRelativeLocation(Loc);

    CrouchTimeline.PlayFromStart();
}

void ABaseCharacter::OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust)
{
    Super::OnEndCrouch(HalfHeightAdjust, ScaledHalfHeightAdjust);

    // Cancel the instant camera rise caused by capsule expanding
    CurrentCrouchCompZ -= HalfHeightAdjust;

    CrouchFromZ = CurrentCrouchCompZ;
    CrouchToZ = 0.f;

    FVector Loc = FpsPivot->GetRelativeLocation();
    Loc.Z = BasePivotFpsZ + CurrentCrouchCompZ;
    FpsPivot->SetRelativeLocation(Loc);

    CrouchTimeline.PlayFromStart(); // forward again (not Reverse)
}

void ABaseCharacter::HandleCrouchProgress(float Alpha)
{
    CurrentCrouchCompZ = FMath::Lerp(CrouchFromZ, CrouchToZ, Alpha);

    FVector Loc = FpsPivot->GetRelativeLocation();
    Loc.Z = BasePivotFpsZ + CurrentCrouchCompZ;
    FpsPivot->SetRelativeLocation(Loc);
}

                                                                                                                         
void ABaseCharacter::ChangeView()
{
	if (CameraComp) {
        CameraComp->ToggleView();
	}
}


USkeletalMeshComponent* ABaseCharacter::GetCurrentMesh() const
{
    if (!CameraComp) {
		return GetMesh();
    }
    if (CameraComp->IsFPS())
    {
        return MeshFps;
    }
    else
    {
        return GetMesh();
    }
}

void ABaseCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ABaseCharacter, bIsAiming);
    DOREPLIFETIME_CONDITION(
        ABaseCharacter,
        CurrentMovementState,
        COND_SkipOwner
    );
}

EWeaponTypes ABaseCharacter::GetWeaponType() const
{
    if (WeaponComp) {
        return WeaponComp->GetCurrentWeaponType();
    }
	return EWeaponTypes::Unarmed;
}

EWeaponSubTypes ABaseCharacter::GetWeaponSubType() const
{
    if (WeaponComp) {
        return WeaponComp->GetCurrentWeaponSubType();
    }
    return EWeaponSubTypes::None;
}


void ABaseCharacter::RequestStartAiming()
{
    if (!WeaponComp->CanWeaponAim()) {
        return;
    }
    if (bIsAiming) {
        return;
    }
    
	// if is locally controlled, update immediately
    if (IsLocallyControlled()) {
		// Predictive update
		bIsAiming = true;
        ApplyAimingVisuals();
        UpdateMaxWalkSpeed();
	}

    ServerSetAiming(true);
}

void ABaseCharacter::RequestStopAiming()
{
    if (!bIsAiming) {
        return;
    }
  
    // if is locally controlled, update immediately
    if (IsLocallyControlled()) {
        bIsAiming = false;
        ApplyAimingVisuals();
        UpdateMaxWalkSpeed();
    }
    ServerSetAiming(false);
}


void ABaseCharacter::OnRep_IsAiming()
{
    if (IsLocallyControlled()) {
		return; // already handled locally
	}
	UE_LOG(LogTemp, Warning, TEXT("OnRep_IsAiming: %s"), bIsAiming ? TEXT("true") : TEXT("false"));
    ApplyAimingVisuals();
}

void ABaseCharacter::ServerSetAiming_Implementation(bool bNewAiming)
{
    if (!IsAlive()) {
        return;
    }

    if (!WeaponComp) {
        return;
    }
    if (bNewAiming && !WeaponComp->CanWeaponAim()) {
        return;
	}

    if (bIsAiming == bNewAiming) {
        return; // no change
    }

    if (bIsAiming) {
        if (WeaponComp->CanWeaponAim()) {
			AimSensitivity = 0.2f;
        }
    }
    else {
        AimSensitivity = 1.0f;
	}

    bIsAiming = bNewAiming;
    UpdateMaxWalkSpeed();
}


float ABaseCharacter::GetSpeedWalkRatio() const
{
    float Ratio = 1.0f;

    if (WeaponComp)
    {
        const EWeaponTypes WeaponType = WeaponComp->GetCurrentWeaponType();

        switch (WeaponType)
        {
        case EWeaponTypes::Melee:
        case EWeaponTypes::Throwable:
        case EWeaponTypes::Spike:
            Ratio = 1.5f;
            break;

        case EWeaponTypes::Firearm:
        default:
            Ratio = 1.0f;
            break;
        }
    }

    // Aiming penalty
    if (bIsAiming)
    {
        Ratio *= 0.2f;
    }

    return Ratio;
}


void ABaseCharacter::UpdateMaxWalkSpeed() {
    if (IsCrouched()) {
        GetCharacterMovement()->MaxWalkSpeed = CROUCH_WALK_SPEED;
    }
    else if (CurrentMovementState == EMovementState::Slow) {
        GetCharacterMovement()->MaxWalkSpeed = SLOW_WALK_SPEED;
    }
    else {
        GetCharacterMovement()->MaxWalkSpeed = NORMAL_WALK_SPEED * GetSpeedWalkRatio();
    }
}

float ABaseCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    if (HealthComp && HealthComp->IsDead())
    {
        return 0.f; // already dead
	}
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
    // calculate with Armor
    if (!WeaponComp) {
        UE_LOG(LogTemp, Warning, TEXT("WeaponComp is null in TakeDamage"));
        return ActualDamage;
    }
    FArmorState* Armor = WeaponComp->GetArmorState();
    if (Armor)
    {
        if (Armor->ArmorPoints > 0)
        {
            float DamageToHealth = ActualDamage * Armor->ArmorRatio;
            float DamageToArmor = (ActualDamage - DamageToHealth) * Armor->ArmorEfficiency;

            if (DamageToArmor >= Armor->ArmorPoints)
            {
                float Overflow = DamageToArmor - Armor->ArmorPoints;
                DamageToHealth += Overflow;
                Armor->ArmorPoints = 0.0f;
            }
            else
            {
                Armor->ArmorPoints -= DamageToArmor;
            }

            ActualDamage = DamageToHealth;
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("ABaseCharacter::TakeDamage called with DamageAmount: %f"), DamageAmount);
    LastHitByController = EventInstigator;
	bLastHitWasHeadshot = false;

    if (DamageEvent.IsOfType(FMyPointDamageEvent::ClassID))
    {
        const FMyPointDamageEvent* MyEvent =
            static_cast<const FMyPointDamageEvent*>(&DamageEvent);

        EItemId WeaponId = static_cast<EItemId>(MyEvent->WeaponID);

        UE_LOG(LogTemp, Warning, TEXT("Damage came from WeaponId: %d"), (int32)WeaponId);

        UGameManager* GMR = GetWorld()->GetGameInstance()->GetSubsystem<UGameManager>();
        LastDamageCauser = GMR->GetWeaponDataById(WeaponId);
		bLastHitWasHeadshot = MyEvent->bIsHeadshot;
    }
    else if (DamageCauser) {
		AThrownProjectile* Projectile = Cast<AThrownProjectile>(DamageCauser);
        if (Projectile) {
			LastDamageCauser = Projectile->GetWeaponData();
        }
    }
    if (!HealthComp) {
        UE_LOG(LogTemp, Warning, TEXT("HealthComp is null in TakeDamage"));
		return ActualDamage;
	}

    HealthComp->ApplyDamage(ActualDamage);
    ClientPlayHitEffect();

	// if bot, notify AI perception
    if (IsBot()) {
        if (ABotAIController* AICon = Cast<ABotAIController>(GetController()))
        {
            AICon->SetFocus(EventInstigator);
        }
    }
    return ActualDamage;
}


void ABaseCharacter::OnMeleeNotify()
{
    if (WeaponComp)
    {
        WeaponComp->PerformMeleeAttack(0);
    }
}


void ABaseCharacter::OnNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload)
{
	UE_LOG(LogTemp, Warning, TEXT("OnNotifyBegin called with NotifyName: %s"), *NotifyName.ToString());
    if (NotifyName == "MeleeAttack")
    {
        OnMeleeNotify();
    }
    else if (NotifyName == "Notify_GrabMag")
    {
        if (WeaponComp)
        {
            WeaponComp->OnNotifyGrabMag();
        }
	}
    else if (NotifyName == "Notify_InsertMag")
    {
        if (WeaponComp)
        {
            WeaponComp->OnNotifyInsertMag();
        }
	}
}

// This function called on server when health reaches zero
void ABaseCharacter::HandleDeath()
{
    // Play animation, ragdoll, notify game mode, etc.
	UE_LOG(LogTemp, Warning, TEXT("Character has died."));
    if (HasAuthority())
    {
        GetCharacterMovement()->StopMovementImmediately();
        GetCharacterMovement()->DisableMovement();

        // get game mode and notify
        if (AShooterGameMode* GM = Cast<AShooterGameMode>(UGameplayStatics::GetGameMode(this)))
        {
            GM->NotifyPlayerKilled(LastHitByController.Get(), GetController(), LastDamageCauser.Get(), bLastHitWasHeadshot);
        }
        LastHitByController = nullptr; // reset after use
        LastDamageCauser = nullptr; // reset after use

        GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        GetMesh()->SetSimulatePhysics(true);
        GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));

 
        MulticastHandleDeath();

        AAIController* AI = Cast<AAIController>(GetController());
        if (AI)
        {
            UBrainComponent* Brain = AI->GetBrainComponent();
            if (Brain)
            {
                Brain->StopLogic("Bot died");
            }
        }

        if (WeaponComp) {
            WeaponComp->OnOwnerDeath();
        }
		SetLifeSpan(10.f); // auto destroy after 5 seconds
    }
}

void ABaseCharacter::MulticastHandleDeath_Implementation()
{
	// if local player, show death UI, etc.
    // change to tps mmode
    if (IsLocallyControlled()) {
        AMyPlayerController* PC = Cast<AMyPlayerController>(GetController());
        if (!PC) {
            return;
        }
        PC->SetIgnoreLookInput(true);
        PC->SetIgnoreMoveInput(true);
        
        if (CameraComp->IsFPS()) {
            ChangeView(); // switch to tps view
        }
        if (!DeathCameraProxyClass) return;

        // Use current camera position as start
        const FVector CamLoc = CameraTps->GetComponentLocation();
        const FRotator CamRot = CameraTps->GetComponentRotation();

        // Spawn proxy actor that has physics + camera
        AActor* Proxy = GetWorld()->SpawnActor<AActor>(DeathCameraProxyClass, CamLoc, CamRot);
        if (!Proxy) {
            return;
        }

        // Add impulse so camera feels like it gets knocked down
        if (UPrimitiveComponent* Root = Cast<UPrimitiveComponent>(Proxy->GetRootComponent()))
        {
            Root->AddImpulse(FVector(
                FMath::RandRange(-80.f, 80.f),
                FMath::RandRange(-80.f, 80.f),
                -250.f
            ) * 20.f);
        }

        // Blend view to death camera
        PC->SetViewTargetWithBlend(Proxy, 0.15f, VTBlend_EaseOut);
		UE_LOG(LogTemp, Warning, TEXT("Switched to death camera proxy."));

		// view spectator mode
        FTimerHandle TmpTimer;
        GetWorld()->GetTimerManager().SetTimer(
            TmpTimer,
            [PC]()
            {
                PC->ServerSetSpectateTarget(false);
            },
            2.0f,
            false
        );
    }
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    //GetMesh()->SetSimulatePhysics(true);
    GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
}

void ABaseCharacter::ServerRevive_Implementation()
{
    // Authoritative state changes
    if (UHealthComponent* HC = FindComponentByClass<UHealthComponent>())
        HC->SetHealth(HC->GetMaxHealth());

    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    GetMesh()->SetSimulatePhysics(false);
    GetMesh()->SetCollisionProfileName(TEXT("CharacterMesh"));

    MulticastReviveFX(); // optional visuals for all
}

void ABaseCharacter::MulticastReviveFX_Implementation()
{
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    GetMesh()->SetSimulatePhysics(false);
    GetMesh()->SetCollisionProfileName(TEXT("CharacterMesh"));
}

void ABaseCharacter::ClientPlayHitEffect_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("ClientPlayHitEffect called"));

	OnHit.Broadcast();
}

void ABaseCharacter::PlayBloodFx(const FVector& HitLocation)
{
    if (IsFpsViewMode()) {
		return; // no blood fx in fps mode
    }
    if (BloodFx) {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            BloodFx,
            HitLocation
        );
    }
}

void ABaseCharacter::PlayStunEffect(const float& Strength) 
{
    if (StunCurve && FlashCollection)
    {
        float NewDuration = BaseStunDuration * Strength;
        // Restart the timeline from the beginning
        StunTimeline.SetPlayRate(BaseStunDuration / FMath::Max(NewDuration, 0.01f));
        StunTimeline.PlayFromStart();
    }
}

void ABaseCharacter::OnStunTimelineUpdate(float Value)
{
    // Value comes from StunCurve (e.g. 1 -> 0 over time)
    if (FlashCollection)
    {
        UWorld* World = GetWorld();
        if (!World) {
            return;
        }

        UMaterialParameterCollectionInstance* MPCInstance = World->GetParameterCollectionInstance(FlashCollection);
        if (MPCInstance)
        {
            // Multiply by StunStrength if you want stronger/weaker stun
            const float Intensity = Value * 1;

            // Set scalar parameter in the post-process MPC
            MPCInstance->SetScalarParameterValue(FName("Intensity"), Intensity);
        }
    }
}

void ABaseCharacter::OnStunTimelineFinished()
{
    // Ensure intensity is fully reset to 0 at the end
    if (FlashCollection)
    {
        if (UMaterialParameterCollectionInstance* MPCInstance = GetWorld()->GetParameterCollectionInstance(FlashCollection))
        {
            MPCInstance->SetScalarParameterValue(FName("Intensity"), 0.0f);
        }
    }
}

float ABaseCharacter::GetAimSensitivity() const {
    return AimSensitivity;
}

void ABaseCharacter::OnPlantSpikeStarted() {
    // play sound
    if (AudioComp) {
        AudioComp->PlayPlantSpike();
    }
}

void ABaseCharacter::OnPlantSpikeStopped() {
    if (AudioComp) {
        AudioComp->StopPlantSpike();
	}
}

void ABaseCharacter::OnDefuseSpikeStarted() {
    if (AudioComp) {
        AudioComp->PlayDefuseSpike();
	}
}

void ABaseCharacter::OnDefuseSpikeStopped() {
    if (AudioComp) {
        AudioComp->StopDefuseSpike();
    }
}

void ABaseCharacter::Destroyed()
{
    Super::Destroyed();

    if (WeaponComp && WeaponComp->GetCurrentWeapon())
    {
        WeaponComp->GetCurrentWeapon()->Destroy();
    }
}

void ABaseCharacter::ServerSetIsSlow_Implementation(bool bNewIsSlow)
{
    if (!IsAlive())
    {
        return;
    }
	UE_LOG(LogTemp, Warning, TEXT("ServerSetIsSlow called with bNewIsSlow: %s"), bNewIsSlow ? TEXT("true") : TEXT("false"));
	CurrentMovementState = bNewIsSlow ? EMovementState::Slow : EMovementState::Normal;
	
    UpdateMaxWalkSpeed();
}

void ABaseCharacter::OnRep_CurrentMovementState()
{
    if (IsLocallyControlled()) {
		return; // already handled locally
    }
    UpdateMaxWalkSpeed();
}

void ABaseCharacter::PlayFootstepSound()
{
    if (!AudioComp) {
        UE_LOG(LogTemp, Warning, TEXT("AudioComp is null in PlayFootstepSound"));
        return;
    }

    const float Speed = GetVelocity().Size2D();
    const bool bGrounded = !GetCharacterMovement()->IsFalling();

    // OFF when slow
    if (Speed < ABaseCharacter::FOOTSTEP_SPEED_MIN)
        return;

    if (!bGrounded)
        return;

    // Play at actor's feet
	AudioComp->PlayFootstep();
}

bool ABaseCharacter::IsAlive() const
{
    if (HealthComp)
    {
        return !HealthComp->IsDead();
    }
    return true; // if no health component, assume alive
}

void ABaseCharacter::Landed(const FHitResult& Hit)
{
    Super::Landed(Hit);

    PlayLandingSound();
}

void ABaseCharacter::PlayLandingSound()
{
    if (AudioComp) {
        AudioComp->PlayLanding();
        return;
    }
}

void ABaseCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
    
    if (UGameManager* GM = UGameManager::Get(GetWorld()))
    {
        GM->UnregisterPlayer(this);
    }
}

void ABaseCharacter::BecomeViewTarget(APlayerController* PC)
{
    Super::BecomeViewTarget(PC);

    if (!PC->IsLocalController()) {
        UE_LOG(LogTemp, Warning, TEXT("ABaseCharacter::BecomeViewTarget called - NO LOCAL"));
        return;
    }

    AMyPlayerController* MyPC = Cast<AMyPlayerController>(PC);
    if (!MyPC)
    {
        UE_LOG(LogTemp, Error, TEXT("PC is not AMyPlayerController in BecomeViewTarget"));
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("DEUBGGGG: ABaseCharacter : BecomeViewTarget"));

    if (IsValid(CameraComp)) {
		UE_LOG(LogTemp, Warning, TEXT("DEUBGGGG: CameraComp is valid in BecomeViewTarget"));
        CameraComp->OnBecomeViewTarget(MyPC);
    }
    else {
        UE_LOG(LogTemp, Warning, TEXT("DEUBGGGG: CameraComp is null in BecomeViewTarget"));
	}
}

void ABaseCharacter::EndViewTarget(APlayerController* PC)
{
    Super::EndViewTarget(PC);

    if (!IsLocallyControlled() && PC && PC->IsLocalController())
    {
		//SetFpsView(false);
    }
    if (CameraComp) {
        CameraComp->OnEndViewTarget(PC);
	}
}

void ABaseCharacter::ApplyRotationMode(bool bIsPlayer)
{
    auto* Move = GetCharacterMovement();
    if (!Move) return;

    bUseControllerRotationYaw = false; // keep false for smooth turning via MovementComponent

    if (bIsPlayer)
    {
        // Smoothly rotate to controller yaw (camera yaw)
        Move->bOrientRotationToMovement = false;
        Move->bUseControllerDesiredRotation = true;
        Move->RotationRate = FRotator(0.f, 540.f, 0.f);
    }
    else
    {
        // Smoothly rotate to movement direction during MoveTo
        Move->bUseControllerDesiredRotation = false;
        Move->bOrientRotationToMovement = true;
        Move->RotationRate = FRotator(0.f, 360.f, 0.f);
    }
}

void ABaseCharacter::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);
    ApplyRotationMode(Cast<APlayerController>(NewController) != nullptr);
}

void ABaseCharacter::OnRep_Controller()
{
    Super::OnRep_Controller();
    ApplyRotationMode(Cast<APlayerController>(Controller) != nullptr);
}

FVector ABaseCharacter::GetThrowableLocation() const {
    return ThrowableLocation
        ? ThrowableLocation->GetComponentLocation()
        : GetActorLocation();
}

USceneCaptureComponent2D* ABaseCharacter::GetViewmodelCapture() const {
    return ViewmodelCap;
}

UBehaviorTree* ABaseCharacter::GetBehaviorTree() const {
    return BehaviorTree;
}

EMovementState ABaseCharacter::GetCurrentMovementState() const {
    return CurrentMovementState;
}

bool ABaseCharacter::IsFpsViewMode() const {
    if (CameraComp) {
        return CameraComp->IsFPS();
	}
	return false;
}

UPickupComponent* ABaseCharacter::GetPickupComponent() const {
	return PickupComponent.Get();
}

UInventoryComponent* ABaseCharacter::GetInventoryComponent() const {
    return InventoryComp.Get();
}

UWeaponComponent* ABaseCharacter::GetWeaponComponent() const {
    return WeaponComp.Get();
}

UInteractComponent* ABaseCharacter::GetInteractComponent() const {
    return InteractComp.Get();
}

UHealthComponent* ABaseCharacter::GetHealthComponent() const {
    return HealthComp.Get();
}

UAnimationComponent* ABaseCharacter::GetAnimationComponent() const {
    return AnimationComp.Get();
}

USplineComponent* ABaseCharacter::GetThrowSpline() const {
    return ThrowSpline;
}

void ABaseCharacter::RequestSlowMovement(bool bNewIsSlow)
{
    ServerSetIsSlow(bNewIsSlow);

    if (IsLocallyControlled())
    {
        CurrentMovementState = bNewIsSlow
            ? EMovementState::Slow
            : EMovementState::Normal;

        UpdateMaxWalkSpeed();
    }
}

void ABaseCharacter::RequestJump() {
    if (GetCharacterMovement()->IsFalling())
    {
        return;
    }

	Jump();
}

bool ABaseCharacter::IsBot() const
{
    return Controller && Controller->IsA<AAIController>();
}