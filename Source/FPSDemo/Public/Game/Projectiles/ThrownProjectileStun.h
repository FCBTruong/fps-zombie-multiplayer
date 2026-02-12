// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Game/Projectiles/ThrownProjectile.h"
#include "ThrownProjectileStun.generated.h"

/**
 * 
 */
UCLASS()
class FPSDEMO_API AThrownProjectileStun : public AThrownProjectile
{
	GENERATED_BODY()

protected:
	void OnExplode() override;

public:
	virtual void MulticastExplode_Implementation(const FVector& ImpactPoint) override;
};
