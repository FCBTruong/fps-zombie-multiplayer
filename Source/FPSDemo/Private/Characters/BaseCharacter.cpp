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
#include "InputActionValue.h"
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

// Sets default values
ABaseCharacter::ABaseCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    FpsPivot = CreateDefaultSubobject<USceneComponent>(TEXT("FpsPivot"));
    FpsPivot->SetupAttachment(GetRootComponent());
    FpsPivot->bEditableWhenInherited = true;

    CameraFps = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraFps"));
    CameraFps->SetupAttachment(FpsPivot);
    CameraFps->bUsePawnControlRotation = true;
    CameraFps->bEditableWhenInherited = true;

    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBloom"));
    CameraBoom->SetupAttachment(RootComponent);

    CameraTps = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraTps"));
    CameraTps->SetupAttachment(CameraBoom);
    CameraTps->bUsePawnControlRotation = false;

    MeshFps = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshFps"));
    MeshFps->SetupAttachment(FpsPivot);
    MeshFps->bEditableWhenInherited = true;

    ViewmodelCap = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("ViewmodelCap"));
    ViewmodelCap->SetupAttachment(CameraFps);
    ViewmodelCap->bEditableWhenInherited = true;
    ViewmodelCap->bCaptureEveryFrame = true;
    ViewmodelCap->bCaptureOnMovement = true;

    PickupComponent = CreateDefaultSubobject<UPickupComponent>(TEXT("PickupComponent"));
    InventoryComp = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));
    WeaponComp = CreateDefaultSubobject<UWeaponComponent>(TEXT("WeaponComponent"));
    InteractComp = CreateDefaultSubobject<UInteractComponent>(TEXT("InteractComponent"));
	HealthComp = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));
	AnimationComp = CreateDefaultSubobject<UAnimationComponent>(TEXT("AnimationComponent"));

    ThrowSpline = CreateDefaultSubobject<USplineComponent>(TEXT("SplineThrow"));
    ThrowSpline->SetupAttachment(RootComponent);

	ThrowableLocation = CreateDefaultSubobject<USceneComponent>(TEXT("ThrowableLocation"));
	ThrowableLocation->SetupAttachment(RootComponent);

	UE_LOG(LogTemp, Warning, TEXT("ABaseCharacter constructor called"));

    if (!WeaponComp)
    {
        UE_LOG(LogTemp, Error, TEXT("WeaponComp is null in ABaseCharacter constructor"));
	}

    UE_LOG(LogTemp, Warning, TEXT("Binding OnTakeAnyDamage in ABaseCharacter"));

    StimuliSource = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("StimuliSource"));
    StimuliSource->RegisterForSense(UAISense_Sight::StaticClass());
    StimuliSource->RegisterWithPerceptionSystem();

    TargetFOV = DEFAULT_FPS_FOV;
}

// Called when the game starts or when spawned
void ABaseCharacter::BeginPlay()
{
    Super::BeginPlay();

	UGameManager::Get(GetWorld())->RegisterPlayer(this);

    ThrowableLocation = Cast<USceneComponent>(GetDefaultSubobjectByName(TEXT("ThrowableLocation")));

    if (CrouchCurve)
    {
        FOnTimelineFloat ProgressFunction;
        ProgressFunction.BindUFunction(this, FName("HandleCrouchProgress"));
        CrouchTimeline.AddInterpFloat(CrouchCurve, ProgressFunction);
    }


    if (MeshFps) {
        if (UAnimInstance* FPSAnim = MeshFps->GetAnimInstance())
        {
            UE_LOG(LogTemp, Warning, TEXT("FPSAnim is valid in ABaseCharacter"));
            FPSAnim->OnPlayMontageNotifyBegin.AddDynamic(this, &ABaseCharacter::OnNotifyBegin);
        }
    }

    if (UAnimInstance* TPSAnim = GetMesh()->GetAnimInstance())
    {
        TPSAnim->OnPlayMontageNotifyBegin.AddDynamic(this, &ABaseCharacter::OnNotifyBegin);
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

	bHasBeginPlayRun = true;
    if (bRecallBVT_AtBegin)
    {
        UE_LOG(LogTemp, Warning, TEXT("Warning: ABaseCharacter::BeginPlay called before becoming view target"));

        // trigger manually
        APlayerController* PC = Cast<APlayerController>(GetController());
        if (PC)
        {
            BecomeViewTarget(PC); 
        }
		bRecallBVT_AtBegin = false;
    }
}


void ABaseCharacter::OnRep_PlayerState() {
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
    CrouchTimeline.TickTimeline(DeltaTime);
    StunTimeline.TickTimeline(DeltaTime);

    if (CameraFps)
    {
        float CurrentFOV = CameraFps->FieldOfView;
        float NewFOV = FMath::FInterpTo(CurrentFOV, TargetFOV, DeltaTime, 10.f); // 10 = speed
        CameraFps->SetFieldOfView(NewFOV);
    }

    // logic sound
    UpdateFootstepSound(DeltaTime);
}

void ABaseCharacter::UpdateFootstepSound(float DeltaTime) {
    const float Speed = GetVelocity().Size2D();
    const bool bGrounded = !GetCharacterMovement()->IsFalling();

    // stop footstep when slow
    if (Speed < 300.f || !bGrounded)
        return;

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


void ABaseCharacter::UpdateAimingState()
{
    if (IsLocallyControlled()) {
        UE_LOG(LogTemp, Warning, TEXT("Updating Aiming State: %s"), bAiming ? TEXT("Aiming") : TEXT("Not Aiming"));
        // Get Player controller and show scope widget
        AMyPlayerController* PC = Cast<AMyPlayerController>(GetController());

        if (!PC) {
            UE_LOG(LogTemp, Warning, TEXT("PlayerController is null in UpdateAimingState"));
            return;
        }
        if (bAiming)
        {
            if (WeaponComp->IsScopeEquipped()) {
                TargetFOV = 20.f;
                AimSensitivity = 0.2f;

                PC->ShowScope();


                // update speed
                GetCharacterMovement()->MaxWalkSpeed = ABaseCharacter::AIM_WALK_SPEED;
            }
        }
        else
        {
            TargetFOV = ABaseCharacter::DEFAULT_FPS_FOV;
            AimSensitivity = 1.0f;
            PC->HideScope();
            HandleUpdateSpeedWalkCurrently();
        }
    }
}


void ABaseCharacter::StartRunning()
{
	//bHoldingShift = true;
}

void ABaseCharacter::StopRunning()
{
	bHoldingShift = false;
}

void ABaseCharacter::Jump()
{
    if (GetCharacterMovement()->IsFalling()) {
        return;
	}
    if (CurrentMovementState == EMovementState::Crouch)
    {
        CustomUnCrouch();
        return;
	}
    Super::Jump();
}


void ABaseCharacter::StopJumping()
{
    Super::StopJumping();
}

void ABaseCharacter::CustomCrouch()
{
    UE_LOG(LogTemp, Warning, TEXT("Crouch"));
	if (CurrentMovementState == EMovementState::Crouch) return;

    // Tell server to update state
    ServerSetCrouching(true);
}

void ABaseCharacter::CustomUnCrouch()
{
    if (CurrentMovementState != EMovementState::Crouch)
    {
        return;
    }
    UE_LOG(LogTemp, Warning, TEXT("UnCrouch"));
	ServerSetCrouching(false);
}

void ABaseCharacter::ServerSetCrouching_Implementation(bool bNewCrouching)
{
    if (bNewCrouching) {
		if (CurrentMovementState == EMovementState::Crouch) return;

        CurrentMovementState = EMovementState::Crouch;
    }
    else {
        if (CurrentMovementState != EMovementState::Crouch) return;
		CurrentMovementState = EMovementState::Normal;
	}
	bIsCrouching = bNewCrouching;
	

    if (CurrentMovementState == EMovementState::Crouch)
    {
        CrouchTimeline.PlayFromStart();
    }
    else
    {
        CrouchTimeline.Reverse();
    }

	HandleUpdateSpeedWalkCurrently();
}

void ABaseCharacter::HandleCrouchProgress(float Value) {
    GetCapsuleComponent()->SetCapsuleHalfHeight(Value);
    BaseTranslationOffset.Z = -GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	GetMesh()->SetRelativeLocation(BaseTranslationOffset);
}


void ABaseCharacter::RequestCrouch()
{
    if (GetCharacterMovement()->IsFalling()) {
        return;
    }
	
    CustomCrouch();
}

void ABaseCharacter::RequestUnCrouch()
{
    CustomUnCrouch();
}

                                                                                                                         
void ABaseCharacter::ChangeView()
{
	bIsFPS = !bIsFPS;

    UpdateView();
}

void ABaseCharacter::UpdateView()
{
    if (!IsValid(this)) return;
    if (!WeaponComp || IsActorBeingDestroyed()) return;
	UE_LOG(LogTemp, Warning, TEXT("Updating View: %s"), bIsFPS ? TEXT("First Person") : TEXT("Third Person"));

    if (bIsFPS)
    {
        CameraFps->SetActive(true);
     
        CameraTps->SetActive(false);
        GetMesh()->SetOwnerNoSee(true);
        if (MeshFps)
        {
            //MeshFps->SetOwnerNoSee(true);
		}
       
        ViewmodelCap->ShowOnlyComponents.Empty();
        ViewmodelCap->ShowOnlyComponents.AddUnique(MeshFps);
        ViewmodelCap->Activate();

        AMyPlayerController* PC = Cast<AMyPlayerController>(GetController());
        if (PC && PC->IsLocalController()) {
            PC->UpdateViewmodelCapture(true);
        }
    }
    else
    {      
        CameraFps->SetActive(false);
        if (CameraTps)
        {
            CameraTps->SetActive(true);
        }
        GetMesh()->SetOwnerNoSee(false);
        if (MeshFps)
        {
			//MeshFps->SetOwnerNoSee(true);
		}

        ViewmodelCap->ShowOnlyComponents.Empty();
        ViewmodelCap->Deactivate();

		// get controller and update viewmodel
		AMyPlayerController* PC = Cast<AMyPlayerController>(GetController());
        if (PC && PC->IsLocalController()) {
            PC->UpdateViewmodelCapture(false);
        }
    }
    WeaponComp->UpdateAttachLocationWeapon();
}


USkeletalMeshComponent* ABaseCharacter::GetCurrentMesh() const
{
    if (bIsFPS)
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
    DOREPLIFETIME(ABaseCharacter, bAiming);
	DOREPLIFETIME(ABaseCharacter, CurrentMovementState);
	DOREPLIFETIME(ABaseCharacter, bIsCrouching);
}

void ABaseCharacter::DropWeapon()
{
	WeaponComp->DropWeapon();
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


void ABaseCharacter::ClickAim()
{
    if (!WeaponComp->CanWeaponAim()) {
        return;
    }
    if (bAiming) {
        ServerSetAiming(false);
    }
    else {
        ServerSetAiming(true);
    }
	bAiming = !bAiming; // force update for current client

	// if is locally controlled, update immediately
    if (IsLocallyControlled()) {
        UpdateAimingState();
	}
}

void ABaseCharacter::OnRep_IsAiming()
{
    if (IsLocallyControlled()) {
		return; // already handled locally
	}
	UE_LOG(LogTemp, Warning, TEXT("OnRep_IsAiming: %s"), bAiming ? TEXT("true") : TEXT("false"));
    UpdateAimingState();
}

void ABaseCharacter::ServerSetAiming_Implementation(bool bNewAiming)
{
    bAiming = bNewAiming;
    OnRep_IsAiming();
}


float ABaseCharacter::GetSpeedWalkRatio()
{
	EWeaponTypes WeaponType = WeaponComp->GetCurrentWeaponType();
    if (WeaponType == EWeaponTypes::Firearm) {
        return 1;
    }
    else if (WeaponType == EWeaponTypes::Melee) {
        return 1.5f;
    }
    else if (WeaponType == EWeaponTypes::Throwable) {
        return 1.5f;
	}
    else if (WeaponType == EWeaponTypes::Spike) {
        return 1.5f;
    }
    return 1.0f;
}

void ABaseCharacter::HandleUpdateSpeedWalkCurrently() {
    if (CurrentMovementState == EMovementState::Crouch) {
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
    FProofState* Armor = WeaponComp->GetArmorState();
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

    HealthComp->ApplyDamage(ActualDamage);
    ClientPlayHitEffect();

	// if bot, notify AI perception
    if (GetController() && GetController()->IsA<AAIController>()) {
        bIsBot = true;
    }
    if (bIsBot) {
       /* ABotAIController* AICon = Cast<ABotAIController>(GetController());*/
		ABotAIController* AICon = Cast<ABotAIController>(GetController());
        AICon->SetFocus(EventInstigator);
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

		WeaponComp->OnOwnerDeath();
		SetLifeSpan(3.f); // auto destroy after 3 seconds
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
        
        if (bIsFPS) {
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
    GetMesh()->SetSimulatePhysics(true);
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

void ABaseCharacter::SetPosViewmodelCaptureForGun() {
   /* if (ViewmodelCapture) {
		ViewmodelCapture->SetRelativeLocation(ViewmodelCaptureDefaultPos);
		ViewmodelCapture->SetRelativeRotation(ViewmodelCaptureDefaultRot);
	}*/
}

float ABaseCharacter::GetAimSensitivity() {
    return AimSensitivity;
}

void ABaseCharacter::PlayPlantSpikeEffect() {
    // play sound
    if (Sounds.PlantingSpike) {
        if (PlantSpikeAudioComp && PlantSpikeAudioComp->IsPlaying())
            return;
        PlantSpikeAudioComp = UGameplayStatics::SpawnSoundAttached(
            Sounds.PlantingSpike,
            RootComponent
        );
	}
}

void ABaseCharacter::StopPlantSpikeEffect() {
    if (PlantSpikeAudioComp)
    {
        PlantSpikeAudioComp->Stop();
        PlantSpikeAudioComp = nullptr;
    }
}

void ABaseCharacter::PlayDefuseSpikeEffect() {
    // play sound
    if (Sounds.DefusingSpike) {
        if (DefuseSpikeAudioComp && DefuseSpikeAudioComp->IsPlaying())
            return;
        DefuseSpikeAudioComp = UGameplayStatics::SpawnSoundAttached(
            Sounds.DefusingSpike,
            RootComponent
        );
    }
}

void ABaseCharacter::StopDefuseSpikeEffect() {
    if (DefuseSpikeAudioComp)
    {
        DefuseSpikeAudioComp->Stop();
        DefuseSpikeAudioComp = nullptr;
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
	UE_LOG(LogTemp, Warning, TEXT("ServerSetIsSlow called with bNewIsSlow: %s"), bNewIsSlow ? TEXT("true") : TEXT("false"));
	CurrentMovementState = bNewIsSlow ? EMovementState::Slow : EMovementState::Normal;
	
	HandleUpdateSpeedWalkCurrently();
}

void ABaseCharacter::OnRep_CurrentMovementState()
{
    HandleUpdateSpeedWalkCurrently();
}

void ABaseCharacter::PlayFootstepSound()
{
    if (!Sounds.Footstep)
        return;

    const float Speed = GetVelocity().Size2D();
    const bool bGrounded = !GetCharacterMovement()->IsFalling();

    // OFF when slow
    if (Speed < 300.f)
        return;

    if (!bGrounded)
        return;

    // Play at actor's feet
    UGameplayStatics::PlaySoundAtLocation(
        this,
        Sounds.Footstep,
        GetActorLocation()
    );
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
    if (!Sounds.Landing)
        return;
    UGameplayStatics::PlaySoundAtLocation(
        this,
        Sounds.Landing,
        GetActorLocation()
    );
}

void ABaseCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
    
	UGameManager::Get(GetWorld())->UnregisterPlayer(this);
}

void ABaseCharacter::OnRep_IsCrouching()
{
    if (bIsCrouching) {
        CrouchTimeline.PlayFromStart();
    }
    else {
        CrouchTimeline.Reverse();
    }
}

void ABaseCharacter::SetFpsView(bool bNewIsFps)
{
    bIsFPS = bNewIsFps;
    UpdateView();
}


void ABaseCharacter::BecomeViewTarget(APlayerController* PC)
{
    Super::BecomeViewTarget(PC);
    /* if (true) {
         return;
     }*/

    if (!bHasBeginPlayRun)
    {
        bRecallBVT_AtBegin = true;
        // will be called again at BeginPlay
        return;
    }

    UE_LOG(LogTemp, Warning,
        TEXT("BecomeViewTarget: World=%s NetMode=%d PC=%s Local=%d Pawn=%s"),
        *GetWorld()->GetName(),
        (int32)GetWorld()->GetNetMode(),
        *PC->GetPathName(),
        PC->IsLocalController(),
        *GetPathName()
    );
    UE_LOG(LogTemp, Warning, TEXT("ABaseCharacter::BecomeViewTarget called"));
    if (!PC) {
        UE_LOG(LogTemp, Error, TEXT("PC is null in BecomeViewTarget"));
        return;
    }

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

    UE_LOG(LogTemp, Warning, TEXT("ViewmodelCapture is valid in ABaseCharacter"));

    if (!ViewmodelRenderTarget)
    {
        ViewmodelRenderTarget = NewObject<UTextureRenderTarget2D>(this);
        ViewmodelRenderTarget->ClearColor = FLinearColor::Transparent;

        int32 SizeX = 0;
        int32 SizeY = 0;
        PC->GetViewportSize(SizeX, SizeY);

        // Fallback safety
        SizeX = FMath::Max(SizeX, 1);
        SizeY = FMath::Max(SizeY, 1);

        ViewmodelRenderTarget->InitAutoFormat(SizeX, SizeY);
    }
    ViewmodelCap->TextureTarget = ViewmodelRenderTarget;
    ViewmodelCap->Activate();
    if (MaterialOverlayBase && !MaterialOverlayMID)
    {
        MaterialOverlayMID = UMaterialInstanceDynamic::Create(MaterialOverlayBase, this);
        MaterialOverlayMID->SetTextureParameterValue(
            TEXT("ViewmodelTexture"),
            ViewmodelRenderTarget
        );
    }
    if (MaterialOverlayMID)
    {
        MyPC->SetViewmodelOverlay(MaterialOverlayMID);
        MyPC->UpdateViewmodelCapture(true);
    }

    bIsFPS = true;

    UpdateView();
}

void ABaseCharacter::EndViewTarget(APlayerController* PC)
{
    Super::EndViewTarget(PC);

    if (!IsLocallyControlled() && PC && PC->IsLocalController())
    {
		SetFpsView(false);
    }
    ViewmodelCap->ShowOnlyComponents.Empty();
    ViewmodelCap->Deactivate();
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
    return bIsFPS;
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
}

void ABaseCharacter::RequestJump() {
	Jump();
}