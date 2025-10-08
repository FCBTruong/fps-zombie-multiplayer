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
};
