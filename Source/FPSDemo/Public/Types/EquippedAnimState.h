// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */

UENUM(BlueprintType)
enum class EEquippedAnimState : uint8
{
	Unarmed     UMETA(DisplayName = "Unarmed"),
	Rifle       UMETA(DisplayName = "Rifle"),
	Pistol		UMETA(DisplayName = "Pistol"),
	Melee		UMETA(DisplayName = "Melee"),
	Throwable   UMETA(DisplayName = "Throwable"),
	Spike      UMETA(DisplayName = "Spike")
};