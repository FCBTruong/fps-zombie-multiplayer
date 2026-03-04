// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/EngineTypes.h" // ECollisionEnabled, ECollisionChannel, FCollisionResponseContainer
#include "LagCompensationComponent.generated.h"

class ABaseCharacter; // Forward declaration

USTRUCT(BlueprintType)
struct FRewindPhysBodyState
{
	GENERATED_BODY()

	UPROPERTY()
	FName BodyName = NAME_None;

	UPROPERTY()
	FTransform WorldTransform = FTransform::Identity;
};

USTRUCT(BlueprintType)
struct FRewindFrame
{
	GENERATED_BODY()

	UPROPERTY()
	double Time = 0.0;

	UPROPERTY()
	TArray<FRewindPhysBodyState> Bodies;
};

USTRUCT()
struct FCollisionBackup
{
	GENERATED_BODY()

	UPROPERTY()
	bool bHasCapsule = false;

	UPROPERTY()
	bool bHasMesh = false;

	UPROPERTY()
	TEnumAsByte<ECollisionEnabled::Type> CapsuleEnabled = ECollisionEnabled::NoCollision;

	UPROPERTY()
	TEnumAsByte<ECollisionEnabled::Type> MeshEnabled = ECollisionEnabled::NoCollision;

	UPROPERTY()
	TEnumAsByte<ECollisionChannel> MeshObjectType = ECC_Pawn;

	UPROPERTY()
	FCollisionResponseContainer MeshResponses;
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class FPSDEMO_API ULagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	ULagCompensationComponent();
	void Init();

protected:
	virtual void TickComponent(
		float DeltaTime,
		ELevelTick TickType,
		FActorComponentTickFunction* ThisTickFunction
	) override;

public:
	void SaveFrame();

	// Server-only: rewind this character's physics asset bodies, trace, then restore.
	// Returns true if the trace hit this character while rewound.
	bool ConfirmHitRewind(
		ABaseCharacter* Shooter,
		const FVector& TraceStart,
		const FVector& TraceEnd,
		double ShotTime,
		FHitResult& OutHit
	);

	bool GetInterpolatedFrameAtTime(double ShotTime, FRewindFrame& OutFrame) const;
	double GetServerTimeSeconds() const;

private:
	// Adjust to your weapon trace channel later (custom channel recommended)
	UPROPERTY(EditDefaultsOnly, Category = "Lag Compensation")
	TEnumAsByte<ECollisionChannel> WeaponTraceChannel = ECC_Visibility;

	// How much history to keep on the server
	UPROPERTY(EditDefaultsOnly, Category = "Lag Compensation", meta = (ClampMin = "0.05", ClampMax = "1.0"))
	float MaxRecordTime = 0.25f;

	ABaseCharacter* OwnerCharacter = nullptr;

	// Newest frame at index 0
	UPROPERTY()
	TArray<FRewindFrame> FrameHistory;

private:
	void CacheCurrentBodies(FRewindFrame& OutFrame) const;
	void MoveBodiesToFrame(const FRewindFrame& Frame) const;
	void RestoreBodies(const FRewindFrame& CachedFrame) const;
	void EnableRewindCollision(FCollisionBackup& Backup) const;
	void RestoreCollisionFromBackup(const FCollisionBackup& Backup) const;
};