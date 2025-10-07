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

private:
	UWeaponData* Data;
public:	
	// Sets default values for this actor's properties
	AWeaponBase();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	UParticleSystem* MuzzleFlash;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon")
	USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(EditDefaultsOnly, Category = "Weapon")
	TSubclassOf<class ABulletBase> BulletClass;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void PreInitializeComponents() override;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	void OnFire(FVector TargetPoint);
	EWeaponTypes GetWeaponType();
	void InitFromData(class UWeaponData* InData);
	UWeaponData* GetWeaponData() { return Data; };
};
