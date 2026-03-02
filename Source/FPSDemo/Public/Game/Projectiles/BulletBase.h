// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Game/Projectiles/BulletData.h"
#include "BulletBase.generated.h"

UCLASS()
class FPSDEMO_API ABulletBase : public AActor
{
	GENERATED_BODY()

protected:
	UPROPERTY()
	TObjectPtr<UBulletData> Data = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "FX")
	UParticleSystem* ExplosionFX;

	UPROPERTY(EditDefaultsOnly, Category = "FX")
	UMaterialInterface* HitDecal;

	UPROPERTY(EditDefaultsOnly, Category = "FX")
	USoundBase* HitSurfaceSound;
public:	
	// Sets default values for this actor's properties
	ABulletBase();
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	class UProjectileMovementComponent* ProjectileMovement;

	UPROPERTY(VisibleAnywhere, Category = "Mesh")
	UStaticMeshComponent* BulletMesh;

	UPROPERTY(VisibleDefaultsOnly, Category = "Projectile")
	class USphereComponent* CollisionComp;

	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, FVector NormalImpulse,
		const FHitResult& Hit);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	void TraceBehindPawnAndSpawnBloodDecal(const FHitResult& PawnHit);

public:	
	void InitFromData(class UBulletData* InData, FVector FinalDestination);
	void FireTowards(const FVector& Target);
};
