// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/ItemConfig.h"
#include "MeleeConfig.generated.h"

/**
 * 
 */
UCLASS()
class FPSDEMO_API UMeleeConfig : public UItemConfig
{
	GENERATED_BODY()
	
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Melee")
    int32 Damage = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Melee")
    float Range = 150.f;

public:
	virtual EItemType GetItemType() const override { return EItemType::Melee; }
};
