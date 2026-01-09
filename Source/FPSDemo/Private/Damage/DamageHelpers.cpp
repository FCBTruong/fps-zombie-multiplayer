#include "Damage/DamageHelpers.h"

#include "GameFramework/Actor.h"
#include "GameFramework/Controller.h"
#include "Damage/MyDamageType.h"
#include "Damage/MyPointDamageEvent.h"

bool DamageHelpers::IsHeadBone(const FName& BoneName)
{
    if (BoneName.IsNone())
    {
        return false;
    }

    static const TSet<FName> HeadBones = {
        FName("head"),
        FName("neck_01")
    };

    return HeadBones.Contains(FName(*BoneName.ToString().ToLower()));
}

float DamageHelpers::ApplyMyPointDamage(
    AActor* Target,
    const FDamageApplyParams& Params,
    AController* Instigator,
    AActor* DamageCauser
)
{
    if (!Target)
    {
        return 0.f;
    }

    const bool bIsHeadshot =
        Params.bEnableHeadshot &&
        Params.Hit.IsValidBlockingHit() &&
        IsHeadBone(Params.Hit.BoneName);

    float Damage = Params.BaseDamage;
    if (bIsHeadshot)
    {
        Damage *= Params.HeadshotMultiplier;
    }

    FMyPointDamageEvent DamageEvent;
    DamageEvent.DamageTypeClass =
        Params.DamageTypeClass
        ? Params.DamageTypeClass.Get()
        : UMyDamageType::StaticClass();
    DamageEvent.WeaponID = Params.WeaponId;
    DamageEvent.bIsHeadshot = bIsHeadshot;
    DamageEvent.HitInfo = Params.Hit;

    return Target->TakeDamage(Damage, DamageEvent, Instigator, DamageCauser);
}
