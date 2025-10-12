// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/PlayerCharacter.h"

APlayerCharacter::APlayerCharacter() {
	
}

void APlayerCharacter::BeginPlay() {
	Super::BeginPlay();
	if (ScopeWidgetClass) {
		CurrentScopeWidget = CreateWidget<UUserWidget>(GetWorld(), ScopeWidgetClass);
		if (CurrentScopeWidget) {
			CurrentScopeWidget->AddToViewport();
			CurrentScopeWidget->SetVisibility(ESlateVisibility::Hidden);
		}
	}

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
	if (bAiming) {
		ServerSetAiming(false);
	}
	else {
		ServerSetAiming(true);
	}
}

void APlayerCharacter::UpdateAimingState()
{
	UE_LOG(LogTemp, Warning, TEXT("Updating Aiming State: %s"), bAiming ? TEXT("Aiming") : TEXT("Not Aiming"));
    if (bAiming)
    {
        // Smooth FOV zoom
		TargetFOV = 70.f;

		if (WeaponComp->IsScopeEquipped()) {
			TargetFOV = 20.f;
			FirstPersonCamera->SetRelativeLocation(FVector(0.f, 20.f, 0.f));
			AimSensitivity = 0.2f;
			if (CurrentScopeWidget) {
				CurrentScopeWidget->SetVisibility(ESlateVisibility::Visible);
			}
		}
    }
    else
    {
		TargetFOV = 90.f;

		FirstPersonCamera->SetRelativeLocation(FVector(0.f, 0.f, 0.f));
		AimSensitivity = 1.0f;
		if (CurrentScopeWidget) {
			CurrentScopeWidget->SetVisibility(ESlateVisibility::Hidden);
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