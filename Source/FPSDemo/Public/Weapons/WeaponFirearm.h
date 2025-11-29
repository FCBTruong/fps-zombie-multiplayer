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
	void ApplyWeaponData() override;
public:
	AWeaponFirearm();
	virtual bool CanFire() {
		return true;
	};
	void OnFire(FVector TargetPoint) override;
	void PlayOutOfAmmoSound();
	void PlayReloadSound();
	UStaticMeshComponent* MagMesh;
	void AttachMagToDefault();
	int32 GetMaxAmmo() const;
};
