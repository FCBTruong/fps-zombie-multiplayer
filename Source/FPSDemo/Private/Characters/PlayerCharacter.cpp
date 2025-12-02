// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/PlayerCharacter.h"
#include "Controllers/MyPlayerController.h"
#include "GameFramework/CharacterMovementComponent.h"

APlayerCharacter::APlayerCharacter() {
	
}

void APlayerCharacter::BeginPlay() {
	Super::BeginPlay();

	if (WeaponComp) {
		UE_LOG(LogTemp, Warning, TEXT("WeaponComp NOT null"));
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("WeaponComp is NULLLL"));
	}

	if (PickupComponent) {
		UE_LOG(LogTemp, Warning, TEXT("PickupComponent NOT null"));
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("PickupComponent is NULLLL"));
	}
}

void APlayerCharacter::PlayFireRifleMontage(FVector TargetPoint)
{
	// Implement firing animation logic here
	UE_LOG(LogTemp, Warning, TEXT("Playing Fire Rifle Montage"));

	if (FireRifleMontage && GetCurrentMesh() && GetCurrentMesh()->GetAnimInstance())
	{
		GetCurrentMesh()->GetAnimInstance()->Montage_Play(FireRifleMontage);
	}
}

void APlayerCharacter::ClickAim()
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
}

void APlayerCharacter::UpdateAimingState()
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

void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (FirstPersonCamera)
	{
		float CurrentFOV = FirstPersonCamera->FieldOfView;
		float NewFOV = FMath::FInterpTo(CurrentFOV, TargetFOV, DeltaTime, 10.f); // 10 = speed
		FirstPersonCamera->SetFieldOfView(NewFOV);
	}
}

void APlayerCharacter::PlayReloadMontage()
{
	if (ReloadMontage && GetCurrentMesh() && GetCurrentMesh()->GetAnimInstance())
	{
		GetCurrentMesh()->GetAnimInstance()->Montage_Play(ReloadMontage);
	}
}