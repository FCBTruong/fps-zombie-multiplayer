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
	UWeaponData* Data;
	USkeletalMeshComponent* WeaponMesh;
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void PreInitializeComponents() override;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	virtual void OnFire(FVector TargetPoint);
	EWeaponTypes GetWeaponType();
	void InitFromData(class UWeaponData* InData);
	UWeaponData* GetWeaponData() { return Data; };
};
