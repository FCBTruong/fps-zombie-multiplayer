#pragma once
#include "CoreMinimal.h"
#include "Shared/Types/ItemId.h"
#include "PickupData.generated.h"

USTRUCT()
struct FPickupData
{
    GENERATED_BODY()

    UPROPERTY() int32 Id = 0;
    UPROPERTY() EItemId ItemId = EItemId::NONE;
    UPROPERTY() int32 Amount = 0;
	UPROPERTY() int32 AmmoInClip = 0;
	UPROPERTY() int32 AmmoReserve = 0;
    UPROPERTY() FVector_NetQuantize Location;
};
