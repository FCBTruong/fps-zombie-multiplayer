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


// Sets default values
ABaseCharacter::ABaseCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    bCloseToWall = false;
    bReloading = false;
    bEquipped = false;

    PickupComponent = CreateDefaultSubobject<UPickupComponent>(TEXT("PickupComponent"));
    InventoryComp = CreateDefaultSubobject<UInventoryComponent>(TEXT("InventoryComponent"));
    WeaponComp = CreateDefaultSubobject<UWeaponComponent>(TEXT("WeaponComponent"));
    InteractComp = CreateDefaultSubobject<UInteractComponent>(TEXT("InteractComponent"));
	HealthComp = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));

    ThrowSpline = CreateDefaultSubobject<USplineComponent>(TEXT("SplineThrow"));
    ThrowSpline->SetupAttachment(RootComponent);

	UE_LOG(LogTemp, Warning, TEXT("ABaseCharacter constructor called"));

    if (!WeaponComp)
    {
        UE_LOG(LogTemp, Error, TEXT("WeaponComp is null in ABaseCharacter constructor"));
	}

    UE_LOG(LogTemp, Warning, TEXT("Binding OnTakeAnyDamage in ABaseCharacter"));

    StimuliSource = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("StimuliSource"));
    StimuliSource->RegisterForSense(UAISense_Sight::StaticClass());
    StimuliSource->RegisterWithPerceptionSystem();
}

// Called when the game starts or when spawned
void ABaseCharacter::BeginPlay()
{
    Super::BeginPlay();
   
    mesh = GetMesh();

	MeshFps = Cast<USkeletalMeshComponent>(GetDefaultSubobjectByName(TEXT("MeshFps")));
	FirstPersonCamera = Cast<UCameraComponent>(GetDefaultSubobjectByName(TEXT("CameraFps")));
	ThirdPersonCamera = Cast<UCameraComponent>(GetDefaultSubobjectByName(TEXT("CameraTps")));
    
    ThrowableLocation = Cast<USceneComponent>(GetDefaultSubobjectByName(TEXT("ThrowableLocation")));

    if (CrouchCurve)
    {
        FOnTimelineFloat ProgressFunction;
        ProgressFunction.BindUFunction(this, FName("HandleCrouchProgress"));
        CrouchTimeline.AddInterpFloat(CrouchCurve, ProgressFunction);
    }


    if (UAnimInstance* FPSAnim = MeshFps->GetAnimInstance())
    {
		UE_LOG(LogTemp, Warning, TEXT("FPSAnim is valid in ABaseCharacter"));
        FPSAnim->OnPlayMontageNotifyBegin.AddDynamic(this, &ABaseCharacter::OnNotifyBegin);
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

  
	ViewmodelCapture = Cast<USceneCaptureComponent2D>(GetDefaultSubobjectByName(TEXT("ViewmodelCap")));
    if (this->IsLocallyControlled())
    {
        if (FlashCollection)
        {
            if (UMaterialParameterCollectionInstance* MPC =
                GetWorld()->GetParameterCollectionInstance(FlashCollection))
            {
                MPC->SetScalarParameterValue("Intensity", 0.0f);
            }
        }
        if (ViewmodelCapture)
        {
            UE_LOG(LogTemp, Warning, TEXT("ViewmodelCapture is valid in ABaseCharacter"));
			ViewmodelCaptureDefaultPos = ViewmodelCapture->GetRelativeLocation();
			ViewmodelCaptureDefaultRot = ViewmodelCapture->GetRelativeRotation();

            UTextureRenderTarget2D* Texture = NewObject<UTextureRenderTarget2D>(this);
            Texture->InitAutoFormat(1920, 1080);
            Texture->ClearColor = FLinearColor::Transparent;
            ViewmodelCapture->TextureTarget = Texture;
            if (MaterialOverlayBase)
            {
                MaterialOverlayMID = UMaterialInstanceDynamic::Create(MaterialOverlayBase, this);
                MaterialOverlayMID->SetTextureParameterValue("ViewmodelTexture", Texture);

                if (AMyPlayerController* PC = Cast<AMyPlayerController>(GetController()))
                {
                    PC->SetViewmodelOverlay(MaterialOverlayMID);
				}
            }
        }
    }
    else {
        if (ViewmodelCapture)
        {
            ViewmodelCapture->DestroyComponent();
			ViewmodelCapture = nullptr;
		}

        bIsFPS = false;

        if (FirstPersonCamera)
        {
            FirstPersonCamera->DestroyComponent();
            FirstPersonCamera = nullptr;
        }

        if (MeshFps)
        {
            MeshFps->DestroyComponent();
            MeshFps = nullptr;
        }

        if (ThirdPersonCamera) {
            ThirdPersonCamera->DestroyComponent();
            ThirdPersonCamera = nullptr;
        }
    }

    if (this->IsLocallyControlled())
    {
        if (HasAuthority())
        {
            UE_LOG(LogTemp, Warning, TEXT("ABaseCharacter is locally controlled and has authority"));
		}
        bIsFPS = true;
        UE_LOG(LogTemp, Warning, TEXT("ABaseCharacter is locally controlled"));
    }
    UpdateView();
}

void ABaseCharacter::OnRep_PlayerState() {
    // Set mesh based on team
    if (!bAppliedTeamMesh) {
        bAppliedTeamMesh = true;
        SetMeshBaseOnTeam();
	}
}

void ABaseCharacter::SetMeshBaseOnTeam()
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

    if (FirstPersonCamera)
    {
        float CurrentFOV = FirstPersonCamera->FieldOfView;
        float NewFOV = FMath::FInterpTo(CurrentFOV, TargetFOV, DeltaTime, 10.f); // 10 = speed
        FirstPersonCamera->SetFieldOfView(NewFOV);
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
            // Smooth FOV zoom
            TargetFOV = 70.f;

            if (WeaponComp->IsScopeEquipped()) {
                TargetFOV = 20.f;
                FirstPersonCamera->SetRelativeLocation(FVector(15.f, 20.f, 0.f));
                AimSensitivity = 0.2f;

                PC->ShowScope();


                // update speed
                GetCharacterMovement()->MaxWalkSpeed = ABaseCharacter::AIM_WALK_SPEED;
            }
        }
        else
        {
            TargetFOV = 90.f;

            FirstPersonCamera->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
            AimSensitivity = 1.0f;
            PC->HideScope();
            HandleUpdateSpeedWalkCurrently();
        }
    }
}

void ABaseCharacter::PlayFireRifleMontage(FVector TargetPoint)
{
    // Implement firing animation logic here
    UE_LOG(LogTemp, Warning, TEXT("Playing Fire Rifle Montage"));

    if (FireRifleMontage && GetCurrentMesh() && GetCurrentMesh()->GetAnimInstance())
    {
        GetCurrentMesh()->GetAnimInstance()->Montage_Play(FireRifleMontage);
    }
}

void ABaseCharacter::PlayFirePistolMontage(FVector TargetPoint)
{
    // Implement firing animation logic here
    UE_LOG(LogTemp, Warning, TEXT("Playing Fire Pistol Montage"));
    if (FirePistolMontage && GetCurrentMesh() && GetCurrentMesh()->GetAnimInstance())
    {
        GetCurrentMesh()->GetAnimInstance()->Montage_Play(FirePistolMontage);
    }
}

void ABaseCharacter::PlayReloadMontage(UWeaponData* WeaponConf)
{
    if (WeaponConf->WeaponSubType == EWeaponSubTypes::Rifle) {
        if (ReloadMontage && GetCurrentMesh() && GetCurrentMesh()->GetAnimInstance())
        {
            GetCurrentMesh()->GetAnimInstance()->Montage_Play(ReloadMontage);
        }
        UE_LOG(LogTemp, Warning, TEXT("Playing Reload Rifle Montage"));
    }
    else if (WeaponConf->WeaponSubType == EWeaponSubTypes::Pistol) {
        UE_LOG(LogTemp, Warning, TEXT("Playing Reload Pistol Montage"));
        if (ReloadPistolMontage && GetCurrentMesh() && GetCurrentMesh()->GetAnimInstance())
        {
            GetCurrentMesh()->GetAnimInstance()->Montage_Play(ReloadPistolMontage);
        }
    }
}

// Called to bind functionality to input
void ABaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);
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

bool ABaseCharacter::IsRunning()
{
    return false;
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
    CrouchTimeline.PlayFromStart();
}

void ABaseCharacter::CustomUnCrouch()
{
    if (CurrentMovementState != EMovementState::Crouch)
    {
        return;
    }
    UE_LOG(LogTemp, Warning, TEXT("UnCrouch"));
	ServerSetCrouching(false);
	CrouchTimeline.Reverse();
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
	mesh->SetRelativeLocation(BaseTranslationOffset);
}


void ABaseCharacter::Input_Crouch()
{
    if (GetCharacterMovement()->IsFalling()) {
        return;
    }
	
    CustomCrouch();
}

void ABaseCharacter::Input_UnCrouch()
{
    CustomUnCrouch();
}


void ABaseCharacter::AddWeapon(AWeaponBase* NewWeapon)
{
	
}
                                                                                                                          

void ABaseCharacter::ChangeView()
{
	bIsFPS = !bIsFPS;

    UpdateView();
}

void ABaseCharacter::UpdateView()
{
    if (bIsFPS)
    {
        if (FirstPersonCamera)
        {
            FirstPersonCamera->SetActive(true);
            CurrentCamera = FirstPersonCamera;
        }
        if (ThirdPersonCamera)
        {
            ThirdPersonCamera->SetActive(false);
        }
        if (mesh)
        {
            mesh->SetOwnerNoSee(true);
        }
        if (MeshFps)
        {
            MeshFps->SetOwnerNoSee(false);
		}
        if (ViewmodelCapture)
        {
            ViewmodelCapture->ShowOnlyComponents.Empty();
            ViewmodelCapture->ShowOnlyComponents.AddUnique(MeshFps);
			ViewmodelCapture->Activate();
		}
    }
    else
    {
        if (FirstPersonCamera)
        {
            FirstPersonCamera->SetActive(false);
        }
        if (ThirdPersonCamera)
        {
            ThirdPersonCamera->SetActive(true);
            CurrentCamera = ThirdPersonCamera;
        }
        if (mesh)
        {
            mesh->SetOwnerNoSee(false);
        }
        if (MeshFps)
        {
			MeshFps->SetOwnerNoSee(true);
		}

        if (ViewmodelCapture)
        {
			ViewmodelCapture->ShowOnlyComponents.Empty();
        }
    }
    WeaponComp->UpdateAttachLocationWeapon();
}


USkeletalMeshComponent* ABaseCharacter::GetCurrentMesh()
{
    if (bIsFPS)
    {
        return MeshFps;
    }
    else
    {
        return mesh;
    }
}

void ABaseCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ABaseCharacter, bAiming);
	DOREPLIFETIME(ABaseCharacter, CurrentMovementState);
}

void ABaseCharacter::DropWeapon()
{
	WeaponComp->DropWeapon();
}

EWeaponTypes ABaseCharacter::GetWeaponType()
{
    if (WeaponComp) {
        return WeaponComp->GetCurrentWeaponType();
    }
	return EWeaponTypes::Unarmed;
}

EWeaponSubTypes ABaseCharacter::GetWeaponSubType()
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


void ABaseCharacter::PlayEquipWeaponAnimation(EWeaponTypes WeaponType)
{
    if (EquipMontage && GetCurrentMesh() && GetCurrentMesh()->GetAnimInstance())
    {
        GetCurrentMesh()->GetAnimInstance()->Montage_Play(EquipMontage);
    }
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

void ABaseCharacter::OnRepSpeedWalkCurrently()
{
    HandleUpdateSpeedWalkCurrently();
}

float ABaseCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
    AController* EventInstigator, AActor* DamageCauser)
{
    if (HealthComp && HealthComp->IsDead())
    {
        return 0.f; // already dead
	}
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
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
    return ActualDamage;
}

void ABaseCharacter::PlayThrowNadeMontage()
{
    if (!ThrowNadeMontage)
    {
        UE_LOG(LogTemp, Error, TEXT("ThrowNadeMontage is null"));
        return;
    }

	PlayMontage(ThrowNadeMontage);
}

void ABaseCharacter::PlayHoldNadeMontage()
{
    if (!HoldNadeMontage)
    {
        UE_LOG(LogTemp, Error, TEXT("HoldNadeMontage is null"));
        return;
    }
	PlayMontage(HoldNadeMontage);
}

void ABaseCharacter::PlayMontage(UAnimMontage* MontageToPlay)
{
    if (!MontageToPlay)
    {
        UE_LOG(LogTemp, Error, TEXT("MontageToPlay is null"));
        return;
    }
    USkeletalMeshComponent* MeshComp = GetCurrentMesh();
    if (!MeshComp)
    {
        UE_LOG(LogTemp, Error, TEXT("GetCurrentMesh() returned null"));
        return;
    }
    UAnimInstance* AnimInst = MeshComp->GetAnimInstance();
    if (!AnimInst)
    {
        UE_LOG(LogTemp, Error, TEXT("AnimInstance is null"));
        return;
    }
    AnimInst->Montage_Play(MontageToPlay);
}

void ABaseCharacter::PlayMeleeAttackAnimation(int32 AttackIndex) {
	UE_LOG(LogTemp, Warning, TEXT("PlayMeleeAttackAnimation called with AttackIndex: %d"), AttackIndex);
    if (AttackIndex == 0) {
        if (KnifeAttack1Montage) {
			PlayMontage(KnifeAttack1Montage);
        }
    }
    else if (AttackIndex == 1) {
        if (KnifeAttack2Montage) {
            PlayMontage(KnifeAttack2Montage);
        }
    }
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
            GM->NotifyPlayerKilled(LastHitByController, GetController(), LastDamageCauser, bLastHitWasHeadshot);
        }
        if (LastHitByController)
        {
			LastHitByController = nullptr; // reset after use
		}
        if (LastDamageCauser)
		{
			LastDamageCauser = nullptr; // reset after use
		}

        GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        GetMesh()->SetSimulatePhysics(true);
        GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));

        APlayerController* PC = Cast<APlayerController>(GetController());
        if (PC)
        {
            // use PC
        }

        Multicast_HandleDeath();

        AAIController* AI = Cast<AAIController>(GetController());
        if (AI)
        {
            UBrainComponent* Brain = AI->GetBrainComponent();
            if (Brain)
            {
                Brain->StopLogic("Bot died");
            }
        }
    }
}

void ABaseCharacter::Multicast_HandleDeath_Implementation()
{
	// if local player, show death UI, etc.
    // change to tps mmode
    if (IsLocallyControlled()) {
        APlayerController* PC = Cast<APlayerController>(GetController());
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
        const FVector CamLoc = ThirdPersonCamera->GetComponentLocation();
        const FRotator CamRot = ThirdPersonCamera->GetComponentRotation();

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

    Multicast_ReviveFX(); // optional visuals for all
}

void ABaseCharacter::Multicast_ReviveFX_Implementation()
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
    if (ViewmodelCapture) {
		ViewmodelCapture->SetRelativeLocation(ViewmodelCaptureDefaultPos);
		ViewmodelCapture->SetRelativeRotation(ViewmodelCaptureDefaultRot);
	}
}

float ABaseCharacter::GetAimSensitivity() {
    return AimSensitivity;
}

void ABaseCharacter::PlayPlantSpikeEffect() {
    // play sound
    if (PlantingSpikeSound) {
        if (PlantSpikeAudioComp && PlantSpikeAudioComp->IsPlaying())
            return;
        PlantSpikeAudioComp = UGameplayStatics::SpawnSoundAttached(
            PlantingSpikeSound,
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
    if (DefusingSpikeSound) {
        if (DefuseSpikeAudioComp && DefuseSpikeAudioComp->IsPlaying())
            return;
        DefuseSpikeAudioComp = UGameplayStatics::SpawnSoundAttached(
            DefusingSpikeSound,
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
    if (!FootstepCue)
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
        FootstepCue,
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