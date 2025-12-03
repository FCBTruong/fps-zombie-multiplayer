// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Projectiles/ThrownProjectile.h"
#include "ThrownProjectileIncendiary.generated.h"

/**
 * 
 */
UCLASS()
class FPSDEMO_API AThrownProjectileIncendiary : public AThrownProjectile
{
	GENERATED_BODY()
protected:
	void OnExplode() override;

public:
	virtual void MulticastExplode_Implementation(const FVector& ImpactPoint) override;
	void DoDamage();
};
