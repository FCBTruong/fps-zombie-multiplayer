// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseCharacter.h"
#include "UI/ScopeUI.h"
#include "PlayerCharacter.generated.h"

/**
 * 
 */
UCLASS()
class FPSDEMO_API APlayerCharacter : public ABaseCharacter
{
	GENERATED_BODY()
	APlayerCharacter();	
protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	UAnimMontage* FireRifleMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	UAnimMontage* FirePistolMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	UAnimMontage* ReloadMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	UAnimMontage* ReloadPistolMontage;

	float TargetFOV = 90.0f;

public:
	void PlayFireRifleMontage(FVector TargetPoint);
	void PlayFirePistolMontage(FVector TargetPoint);
	void ClickAim() override;
	void StartAiming();
	void StopAiming();
	void UpdateAimingState() override;
	virtual void Tick(float DeltaTime) override;
	void PlayReloadMontage(UWeaponData* WeaponConf) override;
};
