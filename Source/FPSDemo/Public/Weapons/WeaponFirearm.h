// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponBase.h"
#include "WeaponFirearm.generated.h"

/**
 * 
 */
UCLASS()
class FPSDEMO_API AWeaponFirearm : public AWeaponBase
{
	GENERATED_BODY()

protected:
	UPROPERTY(ReplicatedUsing = OnRep_CurrentAmmo)
	int CurrentAmmo = 30;
	UPROPERTY(ReplicatedUsing = OnRep_MaxAmmo)
	int MaxAmmo = 50;
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	UFUNCTION()
	void OnRep_CurrentAmmo();
	UFUNCTION()
	void OnRep_MaxAmmo();
public:
	virtual bool CanFire() {
		return true;
	};
	void OnFire(FVector TargetPoint) override;
	void ConsumeAmmo(int Amount);
	void SetCurrentAmmo(int NewCurrentAmmo);
	bool HasAmmoInClip() const override;
	void PlayOutOfAmmoSound();
	void PlayReloadSound();
};
