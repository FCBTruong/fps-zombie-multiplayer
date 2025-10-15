// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/ItemIds.h"
#include "InventoryItem.generated.h"

USTRUCT(BlueprintType)
struct FInventoryItem
{
    GENERATED_BODY()

    EItemId ItemId = EItemId::NONE;

    int32 Count = 0;

    int32 InventoryId = -1;

    int32 AmmoInMag = 0;
};