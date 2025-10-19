// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "WeaponTypes.h"
#include "WeaponConfig.h"
#include "Items/ItemData.h"
#include "Projectiles/BulletData.h"
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
	FWeaponConfig Config;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	EWeaponTypes WeaponType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	EWeaponSubTypes WeaponSubType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bullet")
	UBulletData* BulletData;



    // Explosion particle effect
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Throwable|Effects")
    UParticleSystem* ExplosionFX;

    // Explosion sound
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Throwable|Effects")
    USoundBase* ExplosionSFX;

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
