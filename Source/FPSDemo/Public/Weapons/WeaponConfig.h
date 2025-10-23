// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponConfig.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct FWeaponConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float FireRate = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	int32 MagSize = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	USoundBase* FireSFX = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	UParticleSystem* MuzzleFX = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	FName MuzzleSocket;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	float ReloadTime = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	int32 Damage = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	FVector EquippedOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	FVector EquippedOffsetFps;
};
