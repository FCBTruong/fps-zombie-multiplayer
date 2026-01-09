// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/EquippedItem.h"
#include "WeaponFirearm.generated.h"

/**
 * 
 */
UCLASS()
class FPSDEMO_API AWeaponFirearm : public AEquippedItem
{
	GENERATED_BODY()

protected:
	void ApplyConfig() override;
	virtual void BeginPlay() override;
public:
	AWeaponFirearm();
	
	void OnFire(const FVector& TargetPoint, bool bCustomStart, const FVector& StartPoint);
	void PlayOutOfAmmoSound();
	void PlayReloadSound();

	UPROPERTY()
	UStaticMeshComponent* MagMesh;
	void AttachMagToDefault();
	int32 GetMaxAmmo() const;
	void Destroyed() override;
	virtual void SetViewFps(bool bFps) override;
};
