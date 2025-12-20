// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class EWeaponActionState : uint8
{
    Idle,

    Equipping,
    Unequipping,

    Firing,
    Reloading,
    Aiming,

    Throwing,
    Melee,

    Planting,
    Defusing,

    Disabled
};