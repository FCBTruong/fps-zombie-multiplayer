// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/WeaponBase.h"
#include "Items/ItemIds.h"
#include "Components/ActorComponent.h"
#include "Net/UnrealNetwork.h"
#include "WeaponComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FPSDEMO_API UWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	bool bIsReloading;
	bool bIsAiming;
	bool bIsFiring;
	bool bIsScopeEquipped;
	AWeaponBase* CurrentWeapon;
	FTimerHandle FireTimerHandle;

	void PlayEffectFire(FVector TargetPoint);
public:	
	// Sets default values for this component's properties
	UWeaponComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void EquipWeapon(AWeaponBase* NewWeapon);
	void OnNewItemPickup(EItemId ItemId);
	EWeaponTypes GetCurrentWeaponType();
	void DropWeapon();
	void RequestFireStart();

	UFUNCTION(Server, Reliable)
	void ServerStartFire();
	void RequestFireStop();
	UFUNCTION(Server, Reliable)
	void ServerStopFire();

	void HandleStartFire();
	void HandleStopFire();
	void OnFire();
	void StartAiming();
	void StartReload();
	bool CanShoot();
	bool IsLocalControl();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlayFireRifle(FVector TargetPoint);

	UFUNCTION(BlueprintPure)
	bool IsScopeEquipped();

	UFUNCTION(Server, Reliable)
	void ServerDropWeapon();

	void HandleDropWeapon();
};
