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
	None	 UMETA(DisplayName = "None"),
	Rifle     UMETA(DisplayName = "Rifle"),
	Sniper       UMETA(DisplayName = "Sniper"),
	Pistol		UMETA(DisplayName = "Pistol"),
	Shotgun		UMETA(DisplayName = "Shotgun"),
	Smg   UMETA(DisplayName = "Smg"),
	Frag   UMETA(DisplayName = "Frag"),
	Smoke   UMETA(DisplayName = "Smoke"),
	Stun   UMETA(DisplayName = "Stun")
};
