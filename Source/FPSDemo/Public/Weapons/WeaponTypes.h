#pragma once

#include "CoreMinimal.h" 

UENUM(BlueprintType)
enum class EWeaponTypes : uint8
{
	Unarmed     UMETA(DisplayName = "Unarmed"),
	Firearm       UMETA(DisplayName = "Firearm"),
	Melee		UMETA(DisplayName = "Melee"),
	Throwable   UMETA(DisplayName = "Throwable"),
	Equipment   UMETA(DisplayName = "Equipment")
};

UENUM(BlueprintType)
enum class EWeaponSubTypes : uint8
{
	None	 UMETA(DisplayName = "None"),
	Rifle     UMETA(DisplayName = "Rifle"),
	Pistol		UMETA(DisplayName = "Pistol"),
	Frag   UMETA(DisplayName = "Frag"),
	Smoke   UMETA(DisplayName = "Smoke"),
	Stun   UMETA(DisplayName = "Stun"),
	Incendiary   UMETA(DisplayName = "Incendiary")
};




