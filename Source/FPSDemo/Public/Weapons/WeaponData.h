// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "WeaponTypes.h"
#include "WeaponConfig.h"
#include "Items/ItemData.h"
#include "Projectiles/BulletData.h"
#include "NiagaraSystem.h"
#include "WeaponData.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class FPSDEMO_API UWeaponData : public UItemData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	EWeaponTypes WeaponType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	EWeaponSubTypes WeaponSubType;

	// Firearm specific config

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firearm")
	UBulletData* BulletData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firearm")
	UStaticMesh* MagMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firearm")
	USoundBase* FireSFX = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firearm")
	UParticleSystem* MuzzleFX = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firearm")
	UNiagaraSystem* MuzzleFlashFX = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firearm")
	FName MuzzleSocket;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firearm")
	float ReloadTime = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firearm")
	int32 Damage = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firearm")
	FVector EquippedOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firearm")
	FVector EquippedOffsetFps;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firearm")
	FVector EquippedOffsetRotationFps;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firearm")
	FVector EquippedOffsetRotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firearm")
	USoundBase* OutOfAmmoSFX = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firearm")
	USoundBase* ReloadSFX = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firearm")
	int32 MaxAmmoInClip = 30;




    // Explosion particle effect
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Throwable|Effects")
    UParticleSystem* ExplosionFX;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Throwable|Effects")
	UNiagaraSystem* SmokeFX;

    // Explosion sound
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Throwable|Effects")
    USoundBase* ExplosionSFX;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Throwable|Effects")
	USoundBase* StunSFX;

    // Decal material for scorch mark, etc.
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Throwable|Decal")
    UMaterialInterface* DecalMat;

    // Decal lifetime (seconds)
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Throwable|Decal")
    float DecalLife = 10.0f;

    // Decal size
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Throwable|Decal")
    FVector DecalSize = FVector(20.0f, 20.0f, 20.0f);

    // Explosion radius
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Throwable")
    float ExplosionRadius = 300.0f;
};
