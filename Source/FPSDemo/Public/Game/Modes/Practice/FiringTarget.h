// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FiringTarget.generated.h"

UCLASS()
class FPSDEMO_API AFiringTarget : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFiringTarget();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual float TakeDamage(
		float DamageAmount,
		struct FDamageEvent const& DamageEvent,
		class AController* EventInstigator,
		AActor* DamageCauser
	) override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	void OnHit();
private:
	FVector InitialLocation;

	UPROPERTY()
	UStaticMeshComponent* TargetMesh;

	// Movement settings
	UPROPERTY(EditAnywhere, Category = "Movement")
	float MoveAmplitude = 100.f;

	UPROPERTY(EditAnywhere, Category = "Movement")
	float MoveSpeed = 2.f;

	bool bIsActive = true;

	FRotator OriginalRotation;
	FRotator HitRotation;

	float RotationDuration = 1.0f;
	float RotationElapsed = 0.0f;
	float RunningTime = 0.f;

	FTimerHandle RotationTimer;
	FTimerHandle ReturnDelayTimer;

	void UpdateRotation();
	void StartReturnRotation();
};
