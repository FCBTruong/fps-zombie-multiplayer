// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class EBotRole : uint8
{
    // DEFENDER
    D_Defuser   UMETA(DisplayName = "Defender - Defuser"),
  
    // ATTACKER
    A_Carrier   UMETA(DisplayName = "Attacker - Carrier"),
	A_FindSpike UMETA(DisplayName = "Attacker - FindSpike"),

    // COMMON
    Scout     UMETA(DisplayName = "Scout")
};
