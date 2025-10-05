#pragma once
#include "CoreMinimal.h"
#include "PickupData.generated.h"

USTRUCT()
struct FPickupData
{
    GENERATED_BODY()

    UPROPERTY() int32 Id = 0;
    UPROPERTY() FName ItemId;
    UPROPERTY() int32 Amount = 0;
    UPROPERTY() FVector_NetQuantize Location;
};
