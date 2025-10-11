// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/PlayerCharacter.h"

void APlayerCharacter::PlayFireRifleMontage(FVector TargetPoint)
{
	// Implement firing animation logic here
	UE_LOG(LogTemp, Warning, TEXT("Playing Fire Rifle Montage"));

	if (FireRifleMontage && GetCurrentMesh() && GetCurrentMesh()->GetAnimInstance())
	{
		GetCurrentMesh()->GetAnimInstance()->Montage_Play(FireRifleMontage);
	}
}

