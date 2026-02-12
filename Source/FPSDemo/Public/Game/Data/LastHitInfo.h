#pragma once

#include "CoreMinimal.h"
#include "LastHitInfo.generated.h"

USTRUCT(BlueprintType)
struct FPSDEMO_API FLastHitInfo
{
    GENERATED_BODY()

    UPROPERTY()
    TWeakObjectPtr<class AController> KillerController;

    UPROPERTY()
    TWeakObjectPtr<class AActor> DamageCauser;

    UPROPERTY()
    bool bHeadshot = false;
};
