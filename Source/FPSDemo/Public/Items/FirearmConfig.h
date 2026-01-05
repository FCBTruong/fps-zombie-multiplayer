// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/ItemConfig.h"
#include "FirearmConfig.generated.h"

UENUM(BlueprintType)
enum class EFirearmType : uint8
{
	Rifle,
	Pistol
};

class USoundBase;
class UParticleSystem;
class UNiagaraSystem;
class UStaticMesh;
class UBulletData;

/**
 * 
 */
UCLASS()
class FPSDEMO_API UFirearmConfig : public UItemConfig
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Firearm")
	EFirearmType FirearmType = EFirearmType::Rifle;

	// ===== Firearm Stats =====
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firearm")
	FName MuzzleSocket = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firearm")
	int32 Damage = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firearm")
	int32 MaxAmmoInClip = 30;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firearm")
	int32 AmmoBonus = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firearm")
	float ReloadTime = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firearm")
	float FireInterval = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firearm")
	bool bHasScopeEquipped = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firearm")
	UBulletData* BulletData = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firearm|Mag")
	UStaticMesh* MagMesh = nullptr;

	// ===== VFX =====
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firearm|FX")
    USoundBase* FireSFX = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firearm|FX")
    UParticleSystem* MuzzleFX = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firearm|FX")
    UNiagaraSystem* MuzzleFlashFX = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firearm|Sound")
	USoundBase* DryFireSound = nullptr;

	// ===== Recoil =====
	// Base accuracy (deg)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firearm|Recoil")
	float BaseDeg = 0.2f;

	// Movement contribution (deg)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firearm|Recoil")
	float MoveAddDeg = 3.0f;

	// Airborne penalty (deg)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firearm|Recoil")
	float AirAddDeg = 4.0f;

	// Burst (continuous fire) contribution (deg)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firearm|Recoil")
	float PerShotAddDeg = 1.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firearm|Recoil")
	float MaxBurstAddDeg = 8.0f;

	// How fast burst spread recovers when time passes (deg/sec)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firearm|Recoil")
	float BurstRecoverDegPerSec = 1.0f;

	// Final clamp (deg)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firearm|Recoil")
	float MaxTotalDeg = 10.0f;

	// Curve exponent for movement (1 = linear, 2 = smoother, 3 = even smoother)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firearm|Recoil")
	float MoveCurveExp = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Firearm|Casing")
	TSubclassOf<AActor> CasingClass = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Firearm|Animation")
	TObjectPtr<UAnimSequenceBase> FireAnimation = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Firearm|Animation")
	TObjectPtr<UAnimMontage> CharFireMontage = nullptr;

public:
	virtual EItemType GetItemType() const override { return EItemType::Firearm; }
};
