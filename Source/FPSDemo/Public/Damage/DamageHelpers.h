#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "Items/ItemIds.h"

class AActor;
class AController;
class UDamageType;

struct FDamageApplyParams
{
    float BaseDamage = 0.f;
    EItemId WeaponId = EItemId::NONE;
    TSubclassOf<UDamageType> DamageTypeClass = nullptr;
    bool bEnableHeadshot = true;
    float HeadshotMultiplier = 4.0f;
    FHitResult Hit;
};

namespace DamageHelpers
{
    bool IsHeadBone(const FName& BoneName);

    float ApplyMyPointDamage(
        AActor* Target,
        const FDamageApplyParams& Params,
        AController* Instigator,
        AActor* DamageCauser
    );
}
