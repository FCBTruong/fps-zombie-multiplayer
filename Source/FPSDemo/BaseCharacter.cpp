#include "BaseCharacter.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"


// Sets default values
ABaseCharacter::ABaseCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    bHoldingShoot = false;
    bRunning = false;
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
}

// Called every frame
void ABaseCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    CrouchTimeline.TickTimeline(DeltaTime);
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
    }
}

void ABaseCharacter::StartFire()
{
    if (bHoldingShoot) {
        return;
    }

    bHoldingShoot = true;

    switch (weaponType)
    {
        case EWeaponTypes::Unarmed:
            break;
        case EWeaponTypes::Rifle:
            UE_LOG(LogTemp, Warning, TEXT("Rifle Fire"));
		    FireRifle();
            GetWorldTimerManager().SetTimer(FireTimerHandle, this, &ABaseCharacter::FireRifle, 0.1f, true);
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
    UE_LOG(LogTemp, Warning, TEXT("DEBUGG12"));

    if (FireRifleMontage && mesh)
    {
        mesh->GetAnimInstance()->Montage_Play(FireRifleMontage);
	}
    return;
}

bool ABaseCharacter::CanShoot()
{
    if (bRunning)   return false;
    if (bCloseToWall) return false;
    if (bReloading)   return false;
    return true;
}

void ABaseCharacter::EquipWeapon()
{
    return;
}

void ABaseCharacter::ChangeViewMode()
{
    return;
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
    // log
    UE_LOG(LogTemp, Warning, TEXT("Start Running"));
    if (GetCharacterMovement()->IsFalling()) return;
    if (bCrouching) return;
    if (moveInput.IsZero()) return;
    if (moveInput.Y != 1.f) return; // only run when moving forward
    bRunning = true;

    GetCharacterMovement()->MaxWalkSpeed = MAX_WALK_SPEED;
}

void ABaseCharacter::StopRunning()
{
    bRunning = false;
    GetCharacterMovement()->MaxWalkSpeed = NORMAL_WALK_SPEED;
}

void ABaseCharacter::Jump()
{
    if (bCrouching)
    {
        CustomUnCrouch();
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
    if (bRunning)
    {
        StopRunning();
    }
    if (bCrouching) return;
    bCrouching = true;
    GetCharacterMovement()->MaxWalkSpeed = CROUCH_WALK_SPEED;

    // update capsule half height with timeline
    PlayCrouchTimeline(true);
    if (mesh)
    {
        UE_LOG(LogTemp, Warning, TEXT("Mesh found and moved!"));
    }
    else {
        UE_LOG(LogTemp, Warning, TEXT("Mesh not found!"));
    }
}

void ABaseCharacter::CustomUnCrouch()
{
    if (!bCrouching) return;
    UE_LOG(LogTemp, Warning, TEXT("UnCrouch"));
    bCrouching = false;
    GetCharacterMovement()->MaxWalkSpeed = NORMAL_WALK_SPEED;
    // update capsule half height with timeline
    PlayCrouchTimeline(false);
}


void ABaseCharacter::HandleCrouchProgress(float Value)
{
    UE_LOG(LogTemp, Warning, TEXT("Crouch Progress: %f"), Value);
    GetCapsuleComponent()->SetCapsuleHalfHeight(Value);
    mesh->SetRelativeLocation(FVector(0.f, 0.f, -Value));
}

void ABaseCharacter::PlayCrouchTimeline(bool bCrouchDown)
{
    if (bCrouchDown)
    {
        CrouchTimeline.PlayFromStart();
    }
    else
    {
        CrouchTimeline.ReverseFromEnd();
    }
}

void ABaseCharacter::ClickCrouch()
{
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
    if (!NewWeapon) return;

	AddWeaponToSlot(NewWeapon, 0); // for simplicity, add to slot 0
	EquipSlot(0); // auto equip
}


void ABaseCharacter::AddWeaponToSlot(AWeaponBase* NewWeapon, int32 SlotIndex)
{
    if (!WeaponSlots.IsValidIndex(SlotIndex)) return;

    WeaponSlots[SlotIndex] = NewWeapon;

    // Hide world pickup
    NewWeapon->SetActorHiddenInGame(true);
    NewWeapon->SetActorEnableCollision(false);

    // Attach to character mesh
    NewWeapon->AttachToComponent(GetMesh(),
        FAttachmentTransformRules::SnapToTargetNotIncludingScale,
        TEXT("WeaponSocket"));
}


void ABaseCharacter::EquipSlot(int32 SlotIndex)
{
    if (!WeaponSlots.IsValidIndex(SlotIndex)) return;

	if (CurrentWeapon == WeaponSlots[SlotIndex]) return; // already equipped

    // Hide current weapon
    if (CurrentWeapon) {
        CurrentWeapon->SetActorHiddenInGame(true);
    }

    // Show new weapon
	CurrentWeapon = WeaponSlots[SlotIndex];
	CurrentWeapon->SetActorHiddenInGame(false);

    CurrentWeapon->AttachToComponent(GetMesh(),
        FAttachmentTransformRules::SnapToTargetNotIncludingScale,
        TEXT("weapon_socket"));

	weaponType = EWeaponTypes::Rifle; // For simplicity, assume rifle
}


void ABaseCharacter::Look(const FInputActionValue& Value)
{
    FVector2D LookAxisVector = Value.Get<FVector2D>();
    if (Controller != nullptr)
    {
		LookInput = LookAxisVector;
        // add yaw and pitch input to controller
        AddControllerYawInput(LookAxisVector.X * AimSensitivity);
        AddControllerPitchInput(LookAxisVector.Y * -1 * AimSensitivity);
    }
}


void ABaseCharacter::ChangeView()
{
	bIsFPS = !bIsFPS;
}