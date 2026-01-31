// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Projectiles/BulletData.h"
#include "Components/SceneCaptureComponent2D.h"
#include "BulletBase.generated.h"

UCLASS()
class FPSDEMO_API ABulletBase : public AActor
{
	GENERATED_BODY()

protected:
	UBulletData* Data;

	UPROPERTY(EditDefaultsOnly, Category = "FX")
	UParticleSystem* ExplosionFX;

	UPROPERTY(EditDefaultsOnly, Category = "FX")
	UMaterialInterface* HitDecal;

	UPROPERTY(EditDefaultsOnly, Category = "FX")
	USoundBase* HitSurfaceSound;

	UPROPERTY(EditDefaultsOnly, Category = "Damage")
	float Damage = 10.f;
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

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	void InitFromData(class UBulletData* InData, FVector FinalDestination);
	void FireTowards(const FVector& Target);
};
