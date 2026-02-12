// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Shared/Data/Items/ItemConfig.h"
#include "NiagaraSystem.h"
#include "ThrowableConfig.generated.h"

/**
 * 
 */
UCLASS()
class FPSDEMO_API UThrowableConfig : public UItemConfig
{
	GENERATED_BODY()
	
public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Throwable|Effects")
    UParticleSystem* ExplosionFX = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Throwable|Effects")
    UNiagaraSystem* SmokeFX = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Throwable|Effects")
    USoundBase* ExplosionSFX = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Throwable|Effects")
    USoundBase* AffectSFX;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Throwable|Decal")
    UMaterialInterface* DecalMat = nullptr;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Throwable|Decal")
    float DecalLife = 10.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Throwable|Decal")
	FVector DecalSize = FVector(16.f, 32.f, 32.f);

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Throwable")
    float ExplosionRadius = 300.f;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Throwable")
    float Damage = 300.f;

public:
	virtual EItemType GetItemType() const override { return EItemType::Throwable; }

};
