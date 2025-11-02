// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Weapons/WeaponData.h"
#include "AThrownProjectile.generated.h"

UCLASS()
class FPSDEMO_API AAThrownProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAThrownProjectile();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY() USphereComponent* Collision;
	UPROPERTY() UProjectileMovementComponent* Projectile;
	UPROPERTY() UStaticMeshComponent* WeaponMesh;
	UWeaponData* Data;
	bool bIsExploded = false;

	UFUNCTION()
	void OnProjectileHit(
		UPrimitiveComponent* HitComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, FVector NormalImpulse,
		const FHitResult& Hit);
	FTimerHandle TimerHandle_Explode;
	void ExplodeNow();
	
public:	
	void InitFromData(UWeaponData* InData);
	void LaunchProjectile(FVector LaunchVelocity, AActor* InstigatorActor);
	UFUNCTION(NetMulticast, Unreliable) void MulticastExplode(const FVector& Location);
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	UFUNCTION(NetMulticast, Unreliable) void MulticastInitData(EItemId ItemId);
};
