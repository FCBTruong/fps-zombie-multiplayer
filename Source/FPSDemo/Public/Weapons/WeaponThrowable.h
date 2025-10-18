// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponBase.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "WeaponThrowable.generated.h"

/**
 * 
 */
UCLASS()
class FPSDEMO_API AWeaponThrowable : public AWeaponBase
{
	GENERATED_BODY()

private:
	UPROPERTY(VisibleAnywhere) USphereComponent* Collision;
	UPROPERTY(VisibleAnywhere) UProjectileMovementComponent* Projectile;
public:
	AWeaponThrowable();
	void OnActivate(const FVector& LaunchVel);
};
