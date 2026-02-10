// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectiles/ThrownProjectile.h"
#include "ThrownProjectileFrag.generated.h"

/**
 * 
 */
UCLASS()
class FPSDEMO_API AThrownProjectileFrag : public AThrownProjectile
{
	GENERATED_BODY()

protected:
	virtual void OnExplode() override;
	virtual void BeginPlay() override;

public:
	virtual void MulticastExplode_Implementation(const FVector& ImpactPoint) override;
};
