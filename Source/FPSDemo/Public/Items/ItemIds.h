// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class EItemId : uint8
{
	NONE = 0 UMETA(DisplayName = "None"),
    PISTOL_PL_14,
    RIFLE_AK_47,
    RIFLE_AKS_74,
    RIFLE_M16A,
    RIFLE_QBZ,
    RIFLE_RUSSIAN_AS_VAL,
	MELEE_KNIFE_BASIC,
    GRENADE_FRAG_BASIC,
    GRENADE_SMOKE,
	GRENADE_STUN,
    SNIPER_BOLT_R
};
