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
#include "Weapons/WeaponActionState.h"
#include "WeaponComponent.generated.h"

class ABaseCharacter;

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnUpdateAmmoState, int, int);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnUpdateGrenades, const TArray<EItemId>&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnUpdateCurrentWeapon, const EItemId&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnUpdateRifleWeapon, const EItemId&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnUpdatePistolWeapon, const EItemId&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnUpdatePlantSpikeState, bool);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnUpdateDefuseSpikeState, bool);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnUpdateArmor, int);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FPSDEMO_API UWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UWeaponComponent();

protected:
	// ==== Component Overrides =====
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;

protected:
	float ThrowAngle = 10.f;
	float GrenadeInitSpeed = 1400.f;
	bool bIsInitialized = false;

	// ===== Replicated Properties =====
	UPROPERTY(ReplicatedUsing = OnRep_ActionState)
	EWeaponActionState ActionState = EWeaponActionState::Idle;
	UPROPERTY(Replicated)
	bool bIsAiming;
	UPROPERTY(ReplicatedUsing = OnRep_HasSpike)
	bool bHasSpike = false;
	UPROPERTY(ReplicatedUsing = OnRep_RifleState)
	FWeaponState RifleState;
	UPROPERTY(ReplicatedUsing = OnRep_PistolState)
	FWeaponState PistolState;
	UPROPERTY(ReplicatedUsing = OnRep_MeleeState)
	FWeaponState MeleeState;
	UPROPERTY(ReplicatedUsing = OnRep_Grenades)
	TArray<EItemId> ThrowablesArray;
	UPROPERTY(ReplicatedUsing = OnRep_ArmorState)
	FArmorState ArmorState;
	UPROPERTY(ReplicatedUsing = OnRep_CurrentWeapon)
	EItemId CurrentWeaponId;
	UPROPERTY()
	TObjectPtr<AWeaponBase> CurrentWeapon; // For client prediction
	UPROPERTY()
	TObjectPtr<ATrajectoryPreview> TrajectoryPreviewRef;
	UPROPERTY()
	FTimerHandle ThrowProjectileTimer;
	UPROPERTY()
	FTimerHandle FireTimerHandle;
	UPROPERTY()
	FTimerHandle SpikePlantTimerHandle;
	UPROPERTY()
	FTimerHandle FireTimer;

protected:
	// ===== Protected API =====
	void PlayEffectFire(FVector TargetPoint);
	bool SetActionState(EWeaponActionState NewState);
	bool CanTransition(EWeaponActionState From, EWeaponActionState To) const;
	void OnActionStateChanged(EWeaponActionState OldState, EWeaponActionState NewState);
	void UpdateStateCurrentWeapon();
	void FinishPlantSpike();
	void InitState();
	void OnFinishedReload();
	bool HasAmmoInClip();
	void FireOnce();
	void GetAim(FVector& OutStart, FVector& OutDir) const;

	// ===== Replication Notifies =====
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastThrowAction(FVector LaunchVelocity);
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastDoMeleeAttack(int AttackIdx);
	UFUNCTION(Server, Reliable)
	void ServerSetIsPriming(bool bNewIsPriming);
	UFUNCTION()
	void OnRep_HasSpike();
	UFUNCTION()
	void OnRep_CurrentWeapon();
	UFUNCTION()
	void OnRep_RifleState();
	UFUNCTION()
	void OnRep_PistolState();
	UFUNCTION()
	void OnRep_MeleeState();
	UFUNCTION()
	void OnRep_ArmorState();
	UFUNCTION()
	void OnRep_ActionState(EWeaponActionState OldState);
	UFUNCTION()
	void OnRep_Grenades();
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastSpikePlanted();
	UFUNCTION(Server, Reliable)
	void ServerStartFire();
	UFUNCTION(Server, Reliable)
	void ServerStopFire();
public:	
	// ===== Public API =====
	void EquipWeapon(EItemId ItemId);
	void OnNewItemPickup(int32 NewInventoryId);
	EWeaponTypes GetCurrentWeaponType();
	EWeaponSubTypes GetCurrentWeaponSubType();
	void DropWeapon();
	UFUNCTION(Server, Reliable)
	void ServerOnFire(const FVector& StartPoint, const FVector& TargetPoint, FName HitBoneName);
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
	void ServerStartDefuseSpike();

	UFUNCTION(Server, Reliable)
	void ServerStopDefuseSpike();

	void RequestStartFire();
	void RequestStopFire();
	void OnInput_StartAttack();
	void OnInput_StopAttack();
	void OnFire();
	void HandleOnFire(const FVector& StartPos, const FVector& TargetPoint, FName HitBoneName);
	void StartAiming();
	void StartReload();
	bool CanShoot();
	bool IsLocalControl();

	UFUNCTION(Server, Reliable) void ServerThrow(FVector LaunchVelocity);

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlayFireRifle(FVector TargetPoint);

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
	bool AddNewWeapon(FPickupData ItemData);

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
	bool CanReload();
	void OnInput_StartPlantSpike();
	void OnInput_StopPlantSpike();
	void OnInput_StartDefuseSpike();
	void OnInput_StopDefuseSpike();
	FOnUpdatePlantSpikeState OnUpdatePlantSpikeState;
	FOnUpdateDefuseSpikeState OnUpdateDefuseSpikeState;
	FOnUpdateArmor OnUpdateArmor;
	void FinishDefuseSpike();
	bool CanPlantSpikeAtCurrentLocation();
	void RefreshOverlapPickupActors();
	AWeaponBase* GetCurrentWeapon() const { return CurrentWeapon; }
	bool IsHasSpike();
	void AutoEquipBestWeapon();
	void OnOwnerDeath();
	FWeaponState* GetRifleState() { return &RifleState; }
	FWeaponState* GetPistolState() {
		return &PistolState;
	}
	FArmorState* GetArmorState() {
		return &ArmorState;
	}
	ABaseCharacter* GetCharacter() const;
};
