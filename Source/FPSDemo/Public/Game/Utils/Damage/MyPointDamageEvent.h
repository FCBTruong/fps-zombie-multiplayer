#pragma once

#include "CoreMinimal.h"
#include "Engine/DamageEvents.h"
#include "Shared/Types/ItemId.h"
#include "MyPointDamageEvent.generated.h"

USTRUCT()
struct FMyPointDamageEvent : public FPointDamageEvent
{
    GENERATED_BODY()

    UPROPERTY()
    EItemId WeaponID = EItemId::NONE;
    
    UPROPERTY()
	bool bIsHeadshot = false;

    UPROPERTY()
	bool bIsPenetrationHit = false;

    FMyPointDamageEvent() {}

    FMyPointDamageEvent(
        float InDamage,
        const FHitResult& InHitInfo,
        const FVector& InShotDirection,
        TSubclassOf<UDamageType> InDamageTypeClass,
        EItemId InWeaponID
    )
        : FPointDamageEvent(InDamage, InHitInfo, InShotDirection, InDamageTypeClass)
        , WeaponID(InWeaponID)
    {
    }

    // Must be unique (different from 0, 1, 2 used by engine)
    static const int32 ClassID = 1001;

    virtual int32 GetTypeID() const override { return ClassID; }

    virtual bool IsOfType(int32 InID) const override
    {
        return InID == ClassID || FPointDamageEvent::IsOfType(InID);
    }
};
