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

    // Add mapping context at runtime
    if (APlayerController* PC = Cast<APlayerController>(Controller))
    {
        if (ULocalPlayer* LP = PC->GetLocalPlayer())
        {
            if (UEnhancedInputLocalPlayerSubsystem* Subsys =
                ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LP))
            {
                if (IMC_FPS)
                {
                    Subsys->AddMappingContext(IMC_FPS, 0);
                }
            }
        }
    }

    if (CrouchCurve)
    {
        FOnTimelineFloat ProgressFunction;
        ProgressFunction.BindUFunction(this, FName("HandleCrouchProgress"));
        CrouchTimeline.AddInterpFloat(CrouchCurve, ProgressFunction);
    }


    GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);


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

    if (IsLocallyControlled())
    {
        if (FlashCollection)
        {
            if (UMaterialParameterCollectionInstance* MPC =
                GetWorld()->GetParameterCollectionInstance(FlashCollection))
            {
                MPC->SetScalarParameterValue("Intensity", 0.0f);
            }
        }
    }
	ViewmodelCapture = Cast<USceneCaptureComponent2D>(GetDefaultSubobjectByName(TEXT("ViewmodelCap")));
    if (this->IsLocallyControlled())
    {
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
    }

    if (this->IsLocallyControlled())
    {
        bIsFPS = true;
        UE_LOG(LogTemp, Warning, TEXT("ABaseCharacter is locally controlled"));
    }
    UpdateView();
}

// Called every frame
void ABaseCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    CrouchTimeline.TickTimeline(DeltaTime);
    StunTimeline.TickTimeline(DeltaTime);
}

// Called to bind functionality to input
void ABaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        if (IA_Shoot)
        {
            EIC->BindAction(IA_Shoot, ETriggerEvent::Started, WeaponComp, &UWeaponComponent::OnLeftClickStart);
			EIC->BindAction(IA_Shoot, ETriggerEvent::Completed, WeaponComp, &UWeaponComponent::OnLeftClickRelease);
        }
        if (IA_Movement)
        {
            EIC->BindAction(IA_Movement, ETriggerEvent::Triggered, this, &ABaseCharacter::Move);
        }
        if (IA_JUMP)
        {
            EIC->BindAction(IA_JUMP, ETriggerEvent::Started, this, &ABaseCharacter::Jump);
            EIC->BindAction(IA_JUMP, ETriggerEvent::Completed, this, &ABaseCharacter::StopJumping);
        }
        if (IA_RUN)
        {
            EIC->BindAction(IA_RUN, ETriggerEvent::Started, this, &ABaseCharacter::StartRunning);
            EIC->BindAction(IA_RUN, ETriggerEvent::Completed, this, &ABaseCharacter::StopRunning);
        }
        if (IA_CROUCH)
        {
            EIC->BindAction(IA_CROUCH, ETriggerEvent::Started, this, &ABaseCharacter::ClickCrouch);
        }
        if (IA_CAMERA)
        {
            EIC->BindAction(IA_CAMERA, ETriggerEvent::Triggered, this, &ABaseCharacter::Look);
        }
        if (IA_CHANGE_VIEW)
        {
            EIC->BindAction(IA_CHANGE_VIEW, ETriggerEvent::Started, this, &ABaseCharacter::ChangeView);
        }
        if (IA_SELECT_FIRST_RIFLE)
        {
            EIC->BindAction(IA_SELECT_FIRST_RIFLE, ETriggerEvent::Started, WeaponComp, &UWeaponComponent::EquipSlot, FGameConstants::SLOT_RIFLE);
		}
        if (IA_SELECT_SECOND_RIFLE)
        {
            //EIC->BindAction(IA_SELECT_SECOND_RIFLE, ETriggerEvent::Started, WeaponComp, &UWeaponComponent::EquipSlot, FGameConstants::SLOT_LONG_GUN_2);
        }
        if (IA_SELECT_MELEE)
        {
            EIC->BindAction(IA_SELECT_MELEE, ETriggerEvent::Started, WeaponComp, &UWeaponComponent::EquipSlot, FGameConstants::SLOT_MELEE);
        }
        if (IA_SELECT_PISTOL)
        {
            EIC->BindAction(IA_SELECT_PISTOL, ETriggerEvent::Started, WeaponComp, &UWeaponComponent::EquipSlot, FGameConstants::SLOT_PISTOL);
        }
        if (IA_SELECT_THROWABLE)
        {
            EIC->BindAction(IA_SELECT_THROWABLE, ETriggerEvent::Started, WeaponComp, &UWeaponComponent::EquipSlot, FGameConstants::SLOT_THROWABLE);
        }
        if (IA_DROP_WEAPON)
        {
            EIC->BindAction(IA_DROP_WEAPON, ETriggerEvent::Started, this, &ABaseCharacter::DropWeapon);
        }
        if (IA_PICKUP)
        {
            // With this line, using Enhanced Input system:
            EIC->BindAction(IA_PICKUP, ETriggerEvent::Started, InteractComp, &UInteractComponent::TryPickup);
        }
        if (IA_AIM)
        {
            EIC->BindAction(IA_AIM, ETriggerEvent::Started, this, &ABaseCharacter::ClickAim);
        }
        if (IA_RELOAD)
        {
            EIC->BindAction(IA_RELOAD, ETriggerEvent::Started, WeaponComp, &UWeaponComponent::StartReload);
		}
    }
}


void ABaseCharacter::Move(const FInputActionValue& Value)
{
    moveInput = Value.Get<FVector2D>();

    if (Controller)
    {
        const FRotator ControlRot = Controller->GetControlRotation();
        const FRotator YawRot(0, ControlRot.Yaw, 0);

        const FVector Forward = FRotationMatrix(YawRot).GetUnitAxis(EAxis::X);
        const FVector Right = FRotationMatrix(YawRot).GetUnitAxis(EAxis::Y);

        AddMovementInput(Forward, moveInput.Y);
        AddMovementInput(Right, moveInput.X);
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
    if (bCrouching)
    {
        CustomUnCrouch();
    }
    Super::Jump();
}

bool ABaseCharacter::IsRunning()
{
    const float Speed = GetVelocity().Size2D();

    return bHoldingShift
        && Speed > NORMAL_WALK_SPEED
        && !bCrouching
        && !GetCharacterMovement()->IsFalling()
        && moveInput.Y > 0.f;   // any forward movement
}



void ABaseCharacter::StopJumping()
{
    Super::StopJumping();
}

void ABaseCharacter::CustomCrouch()
{
    UE_LOG(LogTemp, Warning, TEXT("Crouch"));
    if (IsRunning())
    {
        StopRunning();
    }
    if (bCrouching) return;

    // Tell server to update state
    ServerSetCrouching(true);
}

void ABaseCharacter::CustomUnCrouch()
{
    if (!bCrouching) return;
    UE_LOG(LogTemp, Warning, TEXT("UnCrouch"));
	ServerSetCrouching(false);
}

void ABaseCharacter::ServerSetCrouching_Implementation(bool bNewCrouching)
{
    if (bNewCrouching && IsRunning()) {
        StopRunning();
    }
    if (bCrouching == bNewCrouching) {
        return;
    }

    bCrouching = bNewCrouching;
    OnRep_Crouching(); // apply on server too
}

void ABaseCharacter::OnRep_Crouching()
{
	UE_LOG(LogTemp, Warning, TEXT("OnRep_Crouching: %s"), bCrouching ? TEXT("true") : TEXT("false"));
    GetCharacterMovement()->MaxWalkSpeed = bCrouching ? CROUCH_WALK_SPEED : GetSpeedWalkCurrently();
    if (bCrouching) {
        CrouchTimeline.PlayFromStart();
    }
    else {
        CrouchTimeline.ReverseFromEnd();
    }
}

void ABaseCharacter::HandleCrouchProgress(float Value) {
    GetCapsuleComponent()->SetCapsuleHalfHeight(Value);
    BaseTranslationOffset.Z = -GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	mesh->SetRelativeLocation(BaseTranslationOffset);
}


void ABaseCharacter::ClickCrouch()
{
    if (GetCharacterMovement()->IsFalling()) {
        return;
    }
    if (bCrouching)
    {
        CustomUnCrouch();
    }
    else
    {
        CustomCrouch();
    }
}


void ABaseCharacter::AddWeapon(AWeaponBase* NewWeapon)
{
	
}


void ABaseCharacter::Look(const FInputActionValue& Value)
{
    FVector2D LookAxisVector = Value.Get<FVector2D>();
    if (Controller != nullptr)
    {
		LookInput = LookAxisVector;
        //Server_UpdateLookInput(LookAxisVector);
        // add yaw and pitch input to controller
        AddControllerYawInput(LookInput.X * AimSensitivity * 0.3);
        AddControllerPitchInput(LookInput.Y * -1 * AimSensitivity * 0.3);
    }
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

void ABaseCharacter::Server_UpdateLookInput_Implementation(FVector2D NewLookInput)
{
    LookInput = NewLookInput; 

    AddControllerYawInput(LookInput.X * AimSensitivity);
    AddControllerPitchInput(LookInput.Y * -1 * AimSensitivity);
}

void ABaseCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(ABaseCharacter, LookInput);
    DOREPLIFETIME(ABaseCharacter, bCrouching);
    DOREPLIFETIME(ABaseCharacter, bAiming);
	DOREPLIFETIME(ABaseCharacter, SpeedWalkCurrently);
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


void ABaseCharacter::ClickAim()
{
    
}

void ABaseCharacter::OnRep_IsAiming()
{
    // debug
	UE_LOG(LogTemp, Warning, TEXT("OnRep_IsAiming: %s"), bAiming ? TEXT("true") : TEXT("false"));
    UpdateAimingState();
}

void ABaseCharacter::ServerSetAiming_Implementation(bool bNewAiming)
{
    bAiming = bNewAiming;
    OnRep_IsAiming();
}

void ABaseCharacter::UpdateAimingState()
{
   
}

void ABaseCharacter::PlayEquipWeaponAnimation(EWeaponTypes WeaponType)
{
    if (EquipMontage && GetCurrentMesh() && GetCurrentMesh()->GetAnimInstance())
    {
        GetCurrentMesh()->GetAnimInstance()->Montage_Play(EquipMontage);
    }
}

float ABaseCharacter::GetSpeedWalkCurrently()
{
	return SpeedWalkCurrently;
}

// This function called on server to set new speed
void ABaseCharacter::SetSpeedWalkCurrently(float NewSpeed)
{
    SpeedWalkCurrently = NewSpeed;
    HandleUpdateSpeedWalkCurrently();
}

void ABaseCharacter::HandleUpdateSpeedWalkCurrently() {
    if (!bCrouching) {
        GetCharacterMovement()->MaxWalkSpeed = SpeedWalkCurrently;
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

    if (DamageEvent.IsOfType(FMyPointDamageEvent::ClassID))
    {
        const FMyPointDamageEvent* MyEvent =
            static_cast<const FMyPointDamageEvent*>(&DamageEvent);

        EItemId WeaponId = static_cast<EItemId>(MyEvent->WeaponID);

        UE_LOG(LogTemp, Warning, TEXT("Damage came from WeaponId: %d"), (int32)WeaponId);

        UGameManager* GMR = GetWorld()->GetGameInstance()->GetSubsystem<UGameManager>();
        LastDamageCauser = GMR->GetWeaponDataById(WeaponId);
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
        // get game mode and notify
        if (AShooterGameMode* GM = Cast<AShooterGameMode>(UGameplayStatics::GetGameMode(this)))
        {
            GM->NotifyPlayerKilled(LastHitByController, GetController(), LastDamageCauser);
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
        PC->DisableInput(PC);
        
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