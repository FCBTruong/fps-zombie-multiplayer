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
	virtual void BeginPlay() override;
public:
	AWeaponFirearm();
	virtual bool CanFire() {
		return true;
	};
	virtual void OnFire(const FVector& TargetPoint, bool bCustomStart, const FVector& StartPoint) override;
	void PlayOutOfAmmoSound();
	void PlayReloadSound();
	UStaticMeshComponent* MagMesh;
	void AttachMagToDefault();
	int32 GetMaxAmmo() const;
	void Destroyed() override;
	virtual void SetViewFps(bool bFps) override;
};
