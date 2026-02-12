// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Shared/Data/Items/ItemConfig.h"
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
    float Interval = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Melee")
    float Range = 150.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Melee|Decal")
    UMaterialInterface* HitDecalMat = nullptr;


    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Melee|Animation")
    TObjectPtr<UAnimMontage> Attack1Montage = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Melee|Animation")
    TObjectPtr<UAnimMontage> Attack2Montage = nullptr;
public:
	virtual EItemType GetItemType() const override { return EItemType::Melee; }
};
