// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/WeaponBase.h"
#include "Items/ItemIds.h"
#include "Components/ActorComponent.h"
#include "Net/UnrealNetwork.h"
#include "Game/GameManager.h"
#include "Structs/WeaponRuntimeData.h"
#include "GameConstants.h"
#include "Components/SplineComponent.h"
#include "Projectiles/TrajectoryPreview.h"
#include "Weapons/WeaponFirearm.h"
#include "Weapons/WeaponMelee.h"
#include "Weapons/WeaponThrowable.h"
#include "Weapons/WeaponState.h"
#include "WeaponComponent.generated.h"

class UInventoryComponent;
class ABaseCharacter;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnUpdateAmmoState, int, int);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnUpdateGrenades, const TArray<EItemId>&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnUpdateCurrentWeapon, const EItemId&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnUpdateRifleWeapon, const EItemId&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnUpdatePistolWeapon, const EItemId&);


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FPSDEMO_API UWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UWeaponComponent();

protected:
	UGameManager* GMR;

	UPROPERTY(Replicated)
	bool bIsReloading;

	UPROPERTY(Replicated)
	bool bIsAiming;

	UPROPERTY(Replicated)
	bool bIsFiring;
	bool bIsScopeEquipped;
	bool bIsMeleeAttacking;

	UPROPERTY(ReplicatedUsing = OnRep_IsPriming) 
	bool bIsPriming;
	UFUNCTION(Server, Unreliable) void ServerSetIsPriming(bool bNewIsPriming);

	UFUNCTION() void OnRep_IsPriming();
	bool bIsThrowing;


	FTimerHandle FireTimerHandle;

	void PlayEffectFire(FVector TargetPoint);
	// Called when the game starts
	virtual void BeginPlay() override;

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastThrowAction(FVector LaunchVelocity);
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastDoMeleeAttack(int AttackIdx);

	UInventoryComponent* InventoryComp;

	bool bIsInitialized = false;

	UPROPERTY()
	ATrajectoryPreview* TrajectoryPreviewRef;
	UPROPERTY()
	FTimerHandle ThrowProjectileTimer;
	
	float ThrowAngle = 10.f;
	float GrenadeInitSpeed = 1400.f;
	bool bHasSpike = true;


	ABaseCharacter* Character;
	void InitState();
	void OnFinishedReload();

	UPROPERTY(ReplicatedUsing = OnRep_RifleState)
	FWeaponState RifleState;
	UPROPERTY(ReplicatedUsing = OnRep_PistolState)
	FWeaponState PistolState;
	UPROPERTY(ReplicatedUsing = OnRep_MeleeState)
	FWeaponState MeleeState;
	UPROPERTY(ReplicatedUsing = OnRep_Grenades)
	TArray<EItemId> ThrowablesArray;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentWeapon)
	EItemId CurrentWeaponId;

	// This variable is use for client side only
	AWeaponBase* CurrentWeapon;
	UFUNCTION()
	void OnRep_CurrentWeapon();
	UFUNCTION()
	void OnRep_RifleState();
	UFUNCTION()
	void OnRep_PistolState();
	UFUNCTION()
	void OnRep_MeleeState();
	void UpdateStateCurrentWeapon();
	UFUNCTION()
	void OnRep_Grenades();
public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void EquipWeapon(EItemId ItemId);
	void OnNewItemPickup(int32 NewInventoryId);
	EWeaponTypes GetCurrentWeaponType();
	EWeaponSubTypes GetCurrentWeaponSubType();
	void DropWeapon();
	UFUNCTION(Server, Reliable)
	void ServerOnFire(const FVector& StartPoint, const FVector& TargetPoint, const FString& HitBoneName);
	UFUNCTION(Server, Reliable)
	void ServerDoMeleeAttack(int AttackIdx);

	UFUNCTION(Server, Reliable)
	void ServerStartPlantSpike();

	UFUNCTION(Server, Reliable)
	void ServerStopPlantSpike();

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastStartPlantSpike();

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastStopPlantSpike();

	UFUNCTION(Server, Reliable)
	void ServerDefuseSpike();

	void StartAttack();
	void StopAttack();
	void OnFire();
	void HandleOnFire(const FVector& StartPos, const FVector& TargetPoint, const FString& HitBoneName);
	void StartAiming();
	void StartReload();
	bool CanShoot();
	bool IsLocalControl();

	UFUNCTION(Server, Reliable) void ServerThrow(FVector LaunchVelocity);

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlayFireRifle(FVector TargetPoint);

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastDropWeapon(FPickupData Data);

	bool IsScopeEquipped();

	UFUNCTION(Server, Reliable)
	void ServerDropWeapon();

	void HandleDropWeapon();

	UFUNCTION(Server, Reliable)
	void ServerReload();
	void HandleReload();

	UFUNCTION(Server, Reliable)
	void ServerEquipWeapon(EItemId ItemId);
	void EquipSlot(int32 SlotIndex);

	UFUNCTION()
	void DrawProjectileCurve();

	UFUNCTION()
	void UpdateProjectileCurve();

	UFUNCTION()
	FVector GetVelocityGrenade() const;

	void UpdateAttachLocationWeapon();
	bool CanWeaponAim();
	void OnFinishedThrow();
	void HandleEquipWeapon(EItemId ItemId);
	void PerformMeleeAttack(int AttackIdx);

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastReload();
	void OnNotifyGrabMag();
	void OnNotifyInsertMag();
	void OnNewItemAdded(int32 NewInventoryId);
	AWeaponBase* SpawnWeaponByItemId(EItemId ItemId);
	bool AddNewWeapon(EItemId ItemId);

	bool CanDropWeapon(EItemId ItemId);
	FWeaponState* GetWeaponStateByItemId(EItemId ItemId);
	FOnUpdateAmmoState OnUpdateAmmoState;
	FOnUpdateGrenades OnUpdateGrenades;
	FOnUpdateCurrentWeapon OnUpdateCurrentWeapon;
	TArray<EItemId> GetGrenades() const { return ThrowablesArray; } 
	FOnUpdateRifleWeapon OnUpdateRifleWeapon;
	FOnUpdatePistolWeapon OnUpdatePistolWeapon;
	int GetCurrentAmmoInClip();

	void TriggerUpdateUI();
	bool IsReloading() const { return bIsReloading; }
	bool CanReload();
};
