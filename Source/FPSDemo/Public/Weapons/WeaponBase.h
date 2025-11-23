// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "Weapons/WeaponTypes.h"
#include "Weapons/WeaponData.h"
#include "WeaponBase.generated.h"

UCLASS()
class FPSDEMO_API AWeaponBase : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AWeaponBase();
protected:
	UPROPERTY(ReplicatedUsing = OnRep_WeaponData)
	UWeaponData* Data;
	UFUNCTION()
	void OnRep_WeaponData();

	USkeletalMeshComponent* WeaponMesh;
	UStaticMeshComponent* WeaponStaticMesh;
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void PreInitializeComponents() override;
	virtual void ApplyWeaponData();
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	virtual void OnFire(FVector TargetPoint);
	EWeaponTypes GetWeaponType();
	virtual void InitFromData(class UWeaponData* InData);
	UWeaponData* GetWeaponData() { return Data; };
	virtual bool HasAmmoInClip() const { return false; }
	bool CanDrop() const { return Data ? Data->CanDrop : false; }
};
