#include "Game/Utils/Damage/DamageHelpers.h"

#include "GameFramework/Actor.h"
#include "GameFramework/Controller.h"
#include "Game/Utils/Damage/MyDamageType.h"
#include "Game/Utils/Damage/MyPointDamageEvent.h"

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

bool DamageHelpers::IsBodyBone(const FName& BoneName)
{
    if (BoneName.IsNone())
        return false;

    static const TSet<FName> BodyBones = {
        FName("spine_01"),
        FName("spine_02"),
        FName("spine_03"),
        FName("pelvis")
    };

    return BodyBones.Contains(FName(*BoneName.ToString().ToLower()));
}

float DamageHelpers::ApplyMyPointDamage(
    const FDamageApplyParams& Params,
    AController* Instigator,
    AActor* DamageCauser
)
{
	auto Target = Params.Hit.GetActor();

    const FName Bone = Params.Hit.BoneName;
    const bool bIsHeadshot =
        Params.bEnableHeadshot &&
        Params.Hit.IsValidBlockingHit() &&
        IsHeadBone(Bone);

    float Damage = Params.BaseDamage;

    float HeadshotMultiplier = 1.0f; // Default headshot multiplier
    if (bIsHeadshot)
    {
		HeadshotMultiplier = 5.0f; // Example: 4x damage for headshots
    }
    else if (IsBodyBone(Bone)) {
		HeadshotMultiplier = 2.0f; // Example: 1.5x damage for body shots
    }
    Damage *= HeadshotMultiplier;

    FMyPointDamageEvent DamageEvent;
    DamageEvent.DamageTypeClass =
        Params.DamageTypeClass
        ? Params.DamageTypeClass.Get()
        : UMyDamageType::StaticClass();
    DamageEvent.WeaponID = Params.WeaponId;
    DamageEvent.bIsHeadshot = bIsHeadshot;
    DamageEvent.HitInfo = Params.Hit;
	DamageEvent.bIsPenetrationHit = Params.bIsPenetrationHit;

    return Target->TakeDamage(Damage, DamageEvent, Instigator, DamageCauser);
}
