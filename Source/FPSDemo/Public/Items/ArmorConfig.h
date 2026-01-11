// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/ItemConfig.h"
#include "ArmorConfig.generated.h"

/**
 * 
 */
UCLASS()
class FPSDEMO_API UArmorConfig : public UItemConfig
{
	GENERATED_BODY()
	
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Armor")
    int32 ArmorMaxPoints = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Armor")
    float ArmorEfficiency = 0.4f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Armor")
    float ArmorRatio = 0.3f;

public:
    virtual EItemType GetItemType() const override { return EItemType::Armor; }
};
