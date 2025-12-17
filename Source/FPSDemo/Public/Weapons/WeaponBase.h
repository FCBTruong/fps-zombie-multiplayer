// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Weapons/WeaponTypes.h"
#include "Weapons/WeaponData.h"
#include "Components/SceneCaptureComponent2D.h"
#include "WeaponBase.generated.h"

UCLASS()
class FPSDEMO_API AWeaponBase : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AWeaponBase();
protected:
	UWeaponData* Data;
	bool bIsFpsView;

	USkeletalMeshComponent* WeaponMesh;
	UStaticMeshComponent* WeaponStaticMesh;
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void PreInitializeComponents() override;
	virtual void ApplyWeaponData();

	UPROPERTY()
	TWeakObjectPtr<USceneCaptureComponent2D> ViewmodelCapture;
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	virtual void OnFire(const FVector& TargetPoint, bool bCustomStart = false, const FVector& StartPoint = FVector::ZeroVector); // start point for case if bullet comes out from camera
	EWeaponTypes GetWeaponType();
	virtual void InitFromData(class UWeaponData* InData);
	UWeaponData* GetWeaponData() { return Data; };
	virtual bool HasAmmoInClip() const { return false; }
	bool CanDrop() const { return Data ? Data->CanDrop : false; }

	UMeshComponent* GetWeaponMesh();
	virtual void SetViewFps(bool bIsFps);
	void OnUnequipped();
	void OnEquipped();

	EItemId GetItemId() const { return Data ? Data->Id : EItemId::NONE; }
	void SetViewCapture(USceneCaptureComponent2D* InCapture) { ViewmodelCapture = InCapture; }
};
