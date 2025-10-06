// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "WeaponTypes.h"
#include "WeaponBase.generated.h"

UCLASS()
class FPSDEMO_API AWeaponBase : public AActor
{
	GENERATED_BODY()
	
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
	virtual EWeaponTypes GetWeaponType();
};
