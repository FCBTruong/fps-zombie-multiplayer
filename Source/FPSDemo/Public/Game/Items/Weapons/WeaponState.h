#pragma once

#include "CoreMinimal.h"
#include "Shared/Types/ItemId.h"
#include "WeaponState.generated.h"

USTRUCT(BlueprintType)
struct FPSDEMO_API FWeaponState
{
    GENERATED_BODY()

public:
	UPROPERTY()
    int32 AmmoInClip = 0;

    UPROPERTY()
    int32 AmmoReserve = 0;

    UPROPERTY()
	int32 MaxAmmoInClip = 0;

    UPROPERTY()
    bool bIsEquipped = false;

    UPROPERTY()
    EItemId ItemId = EItemId::NONE;
};


USTRUCT(BlueprintType)
struct FPSDEMO_API FArmorState
{
    GENERATED_BODY()

public:
    UPROPERTY()
    int ArmorPoints = 0;

    UPROPERTY()
    int ArmorMaxPoints = 0;

    UPROPERTY()
    float ArmorRatio = 0;

    UPROPERTY()
    float ArmorEfficiency = 0;
};
