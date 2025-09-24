#include "ABaseCharacter.h"

#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"


// Sets default values
AABaseCharacter::AABaseCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    bHoldingShoot = false;
    weaponType = EWeaponTypes::Unarmed;
    bRunning = false;
    bCloseToWall = false;
    bReloading = false;
    bEquipped = false;
}

// Called when the game starts or when spawned
void AABaseCharacter::BeginPlay()
{
    Super::BeginPlay();
    cameraFps = Cast<UCameraComponent>(GetDefaultSubobjectByName(TEXT("CameraTPS")));
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
void AABaseCharacter::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    CrouchTimeline.TickTimeline(DeltaTime);
}

// Called to bind functionality to input
void AABaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
    {
        if (IA_Shoot)
        {
            EIC->BindAction(IA_Shoot, ETriggerEvent::Started, this, &AABaseCharacter::StartFire);
        }
        if (IA_Movement)
        {
            EIC->BindAction(IA_Movement, ETriggerEvent::Triggered, this, &AABaseCharacter::Move);
        }
        if (IA_JUMP)
        {
			EIC->BindAction(IA_JUMP, ETriggerEvent::Started, this, &AABaseCharacter::Jump);
			EIC->BindAction(IA_JUMP, ETriggerEvent::Completed, this, &AABaseCharacter::StopJumping);
		}
        if (IA_RUN)
        {
			EIC->BindAction(IA_RUN, ETriggerEvent::Started, this, &AABaseCharacter::StartRunning);
			EIC->BindAction(IA_RUN, ETriggerEvent::Completed, this, &AABaseCharacter::StopRunning);
        }
        if (IA_CROUCH)
        {
            EIC->BindAction(IA_CROUCH, ETriggerEvent::Started, this, &AABaseCharacter::ClickCrouch);
        }
    }
}

void AABaseCharacter::StartFire()
{
    bHoldingShoot = true;

    switch (weaponType)
    {
    case EWeaponTypes::Unarmed:
        break;
    case EWeaponTypes::Rifle:
        UE_LOG(LogTemp, Warning, TEXT("Rifle Fire"));
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

void AABaseCharacter::FireRifle()
{
    return;
}

bool AABaseCharacter::CanShoot()
{
    if (bRunning)   return false;
    if (bCloseToWall) return false;
    if (bReloading)   return false;
    return true;
}

void AABaseCharacter::EquipWeapon()
{
    return;
}

void AABaseCharacter::ChangeViewMode()
{
    return;
}

void AABaseCharacter::Move(const FInputActionValue& Value)
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

void AABaseCharacter::StartRunning()
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

void AABaseCharacter::StopRunning()
{
    bRunning = false;
    GetCharacterMovement()->MaxWalkSpeed = NORMAL_WALK_SPEED;
}

void AABaseCharacter::Jump()
{
    if (bCrouching)
	{
        CustomUnCrouch();
	}
    Super::Jump();
}

void AABaseCharacter::StopJumping()
{
    Super::StopJumping();
}

void AABaseCharacter::CustomCrouch()
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
        mesh->SetRelativeLocation(FVector(0.f, 0.f, -50.f));
		UE_LOG(LogTemp, Warning, TEXT("Mesh found and moved!"));
    }
    else {
		UE_LOG(LogTemp, Warning, TEXT("Mesh not found!"));
    }
}

void AABaseCharacter::CustomUnCrouch()
{
    if (!bCrouching) return;
	UE_LOG(LogTemp, Warning, TEXT("UnCrouch"));
    bCrouching = false;
    GetCharacterMovement()->MaxWalkSpeed = NORMAL_WALK_SPEED;
    // update capsule half height with timeline
	PlayCrouchTimeline(false);
}


void AABaseCharacter::HandleCrouchProgress(float Value)
{
	UE_LOG(LogTemp, Warning, TEXT("Crouch Progress: %f"), Value);
    GetCapsuleComponent()->SetCapsuleHalfHeight(Value);
}

void AABaseCharacter::PlayCrouchTimeline(bool bCrouchDown)
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

void AABaseCharacter::ClickCrouch()
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