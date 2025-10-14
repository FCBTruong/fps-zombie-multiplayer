#pragma once

#include "CoreMinimal.h"
#include "Weapons/WeaponData.h"
#include "WeaponRuntimeData.generated.h"

USTRUCT(BlueprintType)
struct FWeaponRuntimeData
{
    GENERATED_BODY()

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Weapon")
    TObjectPtr<UWeaponData> WeaponData = nullptr;
};
