// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Game/Projectiles/ThrownProjectile.h"
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
	const float LifeTime = 6.f; // seconds, should move to config instead of hardcoded
public:
	virtual void MulticastExplode_Implementation(const FVector& ImpactPoint) override;
	void DoDamage();
};
