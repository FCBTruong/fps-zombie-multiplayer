#pragma once

#include "CoreMinimal.h" 

UENUM(BlueprintType)
enum class EWeaponTypes : uint8
{
	Unarmed     UMETA(DisplayName = "Unarmed"),
	Rifle       UMETA(DisplayName = "Rifle"),
	Pistol		UMETA(DisplayName = "Pistol"),
	Melee		UMETA(DisplayName = "Melee"),
	Throwable   UMETA(DisplayName = "Throwable")
};
