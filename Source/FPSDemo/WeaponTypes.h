#pragma once

#include "CoreMinimal.h" 

UENUM(BlueprintType)
enum class EWeaponTypes : uint8
{
	Unarmed     UMETA(DisplayName = "Unarmed"),
	Firearm       UMETA(DisplayName = "Firearm"),
	Melee		UMETA(DisplayName = "Melee"),
	Throwable   UMETA(DisplayName = "Throwable")
};

UENUM(BlueprintType)
enum class EWeaponSubTypes : uint8
{
	Rifle     UMETA(DisplayName = "Rifle"),
	Sniper       UMETA(DisplayName = "Sniper"),
	Pistol		UMETA(DisplayName = "Pistol"),
	Shotgun		UMETA(DisplayName = "Shotgun"),
	Smg   UMETA(DisplayName = "Smg"),
	Grenade   UMETA(DisplayName = "Grenade"),
	Smoke   UMETA(DisplayName = "Smoke")
};
