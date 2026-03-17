#pragma once

#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "Shared/Types/ItemId.h"

class AActor;
class AController;
class UDamageType;

struct FDamageApplyParams
{
    float BaseDamage = 0.f;
    EItemId WeaponId = EItemId::NONE;
    TSubclassOf<UDamageType> DamageTypeClass = nullptr;
    bool bEnableHeadshot = true;
    bool bIsPenetrationHit = false;
    FHitResult Hit;
};

namespace DamageHelpers
{
    bool IsHeadBone(const FName& BoneName);
    bool IsBodyBone(const FName& BoneName);
    float ApplyMyPointDamage(
        const FDamageApplyParams& Params,
        AController* Instigator,
        AActor* DamageCauser
    );
}
