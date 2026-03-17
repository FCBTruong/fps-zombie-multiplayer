// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Game/Items/EquippedItem.h"
#include "WeaponFirearm.generated.h"

struct FBulletImpactData;
/**
 * 
 */
UCLASS()
class FPSDEMO_API AWeaponFirearm : public AEquippedItem
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	void ApplyConfig() override;
	void TraceBehindPawnAndSpawnBloodDecal(FVector PawnHit, FVector Dir);
public:
	AWeaponFirearm();
	
	void OnFire(const TArray<FBulletImpactData>& Impacts, FVector TargetPoint);
	void PlayOutOfAmmoSound();
	void PlayReloadSound();

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> MagMesh = nullptr;
	void AttachMagToDefault();
	int32 GetMaxAmmo() const;
	void Destroyed() override;
	virtual void SetViewFps(bool bFps) override;
};
