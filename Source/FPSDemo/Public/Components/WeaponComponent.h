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

public:	
	// Sets default values for this component's properties
	UWeaponComponent();

protected:
	bool bIsReloading;
	bool bIsAiming;
	bool bIsFiring;
	bool bIsScopeEquipped;

	UPROPERTY(Replicated)
	AWeaponBase* CurrentWeapon;

	FTimerHandle FireTimerHandle;

	void PlayEffectFire(FVector TargetPoint);
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

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

	bool IsScopeEquipped();

	UFUNCTION(Server, Reliable)
	void ServerDropWeapon();

	void HandleDropWeapon();
};
