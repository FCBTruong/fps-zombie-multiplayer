#pragma once

#include "CoreMinimal.h"
#include "Items/ItemIds.h"
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
    bool bIsReloading = false;

    UPROPERTY()
    bool bIsEquipped = false;

    UPROPERTY()
    EItemId ItemId = EItemId::NONE;
};
