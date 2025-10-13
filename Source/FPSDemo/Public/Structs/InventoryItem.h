// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/ItemIds.h"
#include "InventoryItem.generated.h"

USTRUCT(BlueprintType)
struct FInventoryItem
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EItemId ItemId = EItemId::NONE;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Count = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 InventoryId = -1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 AmmoInMag = 0;
};