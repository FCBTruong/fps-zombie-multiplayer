#include "BaseCharacter.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "WeaponKnifeBasic.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ABaseCharacter::ABaseCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    bHoldingShoot = false;
    bCloseToWall = false;
    bReloading = false;
    bEquipped = false;
    WeaponSlots.SetNum(4);
}

// Called when the game starts or when spawned
void ABaseCharacter::BeginPlay()
{
    Super::BeginPlay();
    mesh = GetMesh();

	MeshFps = Cast<USkeletalMeshComponent>(GetDefaultSubobjectByName(TEXT("MeshFps")));
	FirstPersonCamera = Cast<UCameraComponent>(GetDefaultSubobjectByName(TEXT("CameraFps")));
	ThirdPersonCamera = Cast<UCameraComponent>(GetDefaultSubobjectByName(TEXT("CameraTps")));

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

    UpdateView();

    if (KnifeClass)
    {
        FActorSpawnParameters Params;
        Params.Owner = this;
        Params.Instigator = GetInstigator();

        AWeaponBase* Knife = GetWorld()->SpawnActor<AWeaponBase>(
            KnifeClass,
            FVector::ZeroVector,
            FRotator::ZeroRotator,
            Params
        );

        if (Knife)
        {
            AddWeaponToSlot(Knife, 3);
            EquipSlot(3);
        }
    }
}

// Called every frame
void ABaseCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    CrouchTimeline.TickTimeline(DeltaTime);

    if (mesh) {
        float HalfHeight = GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
        mesh->SetRelativeLocation(FVector(0.f, 0.f, -HalfHeight));
    }
}

// Called to bind functionality to input
void ABaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        if (IA_Shoot)
        {
            EIC->BindAction(IA_Shoot, ETriggerEvent::Started, this, &ABaseCharacter::StartFire);
			EIC->BindAction(IA_Shoot, ETriggerEvent::Completed, this, &ABaseCharacter::StopFire);
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
            EIC->BindAction(IA_SELECT_FIRST_RIFLE, ETriggerEvent::Started, this, &ABaseCharacter::EquipSlot, 0);
		}
        if (IA_SELECT_SECOND_RIFLE)
        {
            EIC->BindAction(IA_SELECT_SECOND_RIFLE, ETriggerEvent::Started, this, &ABaseCharacter::EquipSlot, 1);
        }
    }
}

void ABaseCharacter::StartFire()
{
    if (bHoldingShoot) {
        return;
    }

    bHoldingShoot = true;
    float timeBetweenShots = 0.2f; // Example value, adjust as needed

    switch (weaponType)
    {
        case EWeaponTypes::Unarmed:
            break;
        case EWeaponTypes::Rifle:
            UE_LOG(LogTemp, Warning, TEXT("Rifle Fire"));
           
            ServerFire();

            GetWorldTimerManager().SetTimer(FireTimerHandle, this, &ABaseCharacter::ServerFire, timeBetweenShots, true);
            break;
        case EWeaponTypes::Pistol:
            UE_LOG(LogTemp, Warning, TEXT("Pistol Fire"));
            break;
        case EWeaponTypes::Melee:
            UE_LOG(LogTemp, Warning, TEXT("Melee Attack"));
            break;
        case EWeaponTypes::Throwable:
            UE_LOG(LogTemp, Warning, TEXT("Throw Grenade"));
            break;
    }
}

void ABaseCharacter::StopFire()
{
    GetWorldTimerManager().ClearTimer(FireTimerHandle);
    bHoldingShoot = false;
}

void ABaseCharacter::FireRifle()
{
    UE_LOG(LogTemp, Warning, TEXT("DEBUGG1"));
    if (!CanShoot()) {
        GetWorldTimerManager().ClearTimer(FireTimerHandle);
        return;
    }
    // get camera viewpoint
    FVector CameraLocation;
    FRotator CameraRotation;
    Controller->GetPlayerViewPoint(CameraLocation, CameraRotation);
    FVector ShotDirection = CameraRotation.Vector();

    FVector TraceEnd = CameraLocation + (ShotDirection * 10000.f);

    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);

    bool bHit = GetWorld()->LineTraceSingleByChannel(
        Hit,
        CameraLocation,
        TraceEnd,
        ECC_Visibility,
        Params
    );

    FVector TargetPoint = bHit ? Hit.ImpactPoint : TraceEnd;

	if (HasAuthority()) // only server makes changes
    {
        MulticastPlayFireRifle(TargetPoint);
    }
    return;
}

bool ABaseCharacter::CanShoot()
{
    if (IsRunning())   return false;
    if (bCloseToWall) return false;
    if (bReloading)   return false;
    return true;
}

void ABaseCharacter::EquipWeapon()
{
    return;
}

void ABaseCharacter::Move(const FInputActionValue& Value)
{
    moveInput = Value.Get<FVector2D>();
    if (bHoldingShift && moveInput.Y > 0.f) {
        GetCharacterMovement()->MaxWalkSpeed = MAX_WALK_SPEED;
    }
    else {
        GetCharacterMovement()->MaxWalkSpeed = NORMAL_WALK_SPEED;
	}

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
	bHoldingShift = true;
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
    if (bNewCrouching && IsRunning()) StopRunning();
    if (bCrouching == bNewCrouching) return;

    bCrouching = bNewCrouching;
    OnRep_Crouching(); // apply on server copy too
}

void ABaseCharacter::OnRep_Crouching()
{
    GetCharacterMovement()->MaxWalkSpeed = bCrouching ? CROUCH_WALK_SPEED : NORMAL_WALK_SPEED;
    if (bCrouching) {
        CrouchTimeline.PlayFromStart();
    }
    else {
        CrouchTimeline.ReverseFromEnd();
    }
}

void ABaseCharacter::HandleCrouchProgress(float Value) {
    GetCapsuleComponent()->SetCapsuleHalfHeight(Value);
   /* if (mesh) {
        mesh->SetRelativeLocation(FVector(0.f, 0.f, -Value));
    }*/
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
	EWeaponTypes NewWeaponType = NewWeapon->GetWeaponType();
    if (!NewWeapon) return;
    if (NewWeaponType == EWeaponTypes::Rifle) {
        for (int i = 0; i < 2; i++)
        {
            if (WeaponSlots[i] == NewWeapon) return; // already have it
            if (WeaponSlots[i] == nullptr) {
                AddWeaponToSlot(NewWeapon, i);
                if (!CurrentWeapon) {
                    EquipSlot(i); // auto equip if no weapon
                }
                return;
            }
        }
    }
}


void ABaseCharacter::AddWeaponToSlot(AWeaponBase* NewWeapon, int32 SlotIndex)
{
    if (!WeaponSlots.IsValidIndex(SlotIndex)) return;

    WeaponSlots[SlotIndex] = NewWeapon;

    // Hide world pickup
    NewWeapon->SetActorHiddenInGame(true);
    NewWeapon->SetActorEnableCollision(false);

    // Attach to character mesh
    NewWeapon->AttachToComponent(GetCurrentMesh(),
        FAttachmentTransformRules::SnapToTargetNotIncludingScale,
        FName(*GetRifleSocketName()));
}


void ABaseCharacter::EquipSlot(int32 SlotIndex)
{
    if (!WeaponSlots.IsValidIndex(SlotIndex)) return;

	if (CurrentWeapon == WeaponSlots[SlotIndex]) return; // already equipped

	if (!WeaponSlots[SlotIndex]) return; // empty slot

    // Hide current weapon
    if (CurrentWeapon) {
        CurrentWeapon->SetActorHiddenInGame(true);
    }

    // Show new weapon
	CurrentWeapon = WeaponSlots[SlotIndex];
	CurrentWeapon->SetActorHiddenInGame(false);

    CurrentWeapon->AttachToComponent(GetCurrentMesh(),
        FAttachmentTransformRules::SnapToTargetNotIncludingScale,
        FName(*GetRifleSocketName())
    );

    weaponType = CurrentWeapon->GetWeaponType();
}


void ABaseCharacter::Look(const FInputActionValue& Value)
{
    FVector2D LookAxisVector = Value.Get<FVector2D>();
    if (Controller != nullptr)
    {
		LookInput = LookAxisVector;
        //Server_UpdateLookInput(LookAxisVector);
        // add yaw and pitch input to controller
        AddControllerYawInput(LookInput.X * AimSensitivity);
        AddControllerPitchInput(LookInput.Y * -1 * AimSensitivity);
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
    }
    if (CurrentWeapon) {
        CurrentWeapon->AttachToComponent(GetCurrentMesh(),
            FAttachmentTransformRules::SnapToTargetNotIncludingScale,
            FName(*GetRifleSocketName())
        );
        if (bIsFPS) {
			CurrentWeapon->SetActorRelativeLocation(FVector(0.f, 0.f, -6.f));
        }
    }
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

FString ABaseCharacter::GetRifleSocketName()
{
    if (bIsFPS)
    {
        return FString("ik_hand_gun");
    }
    else
    {
        return FString("weapon_socket");
	}
}

void ABaseCharacter::ServerFire_Implementation()
{
	FireRifle();
}


void ABaseCharacter::MulticastPlayFireRifle_Implementation(FVector TargetPoint)
{
    if (FireRifleMontage && GetCurrentMesh() && GetCurrentMesh()->GetAnimInstance())
    {
        GetCurrentMesh()->GetAnimInstance()->Montage_Play(FireRifleMontage);
    }

    if (CurrentWeapon) {
		CurrentWeapon->OnFire(TargetPoint);
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
}
