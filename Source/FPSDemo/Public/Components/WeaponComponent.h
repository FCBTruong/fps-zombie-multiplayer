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
#include "WeaponComponent.generated.h"

class UInventoryComponent;
class ABaseCharacter;

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

	UPROPERTY(ReplicatedUsing = OnRep_IsPriming) 
	bool bIsPriming;
	UFUNCTION(Server, Unreliable) void ServerSetIsPriming(bool bNewIsPriming);

	UFUNCTION() void OnRep_IsPriming();
	bool bIsThrowing;

	// For client only, server DOES NOT use this pointer
	AWeaponBase* CurrentWeapon;

	FTimerHandle FireTimerHandle;

	void PlayEffectFire(FVector TargetPoint);
	// Called when the game starts
	virtual void BeginPlay() override;

	UPROPERTY(ReplicatedUsing=OnRep_CurrentInventoryId)
	int32 CurrentInventoryId;

	UFUNCTION()
	void OnRep_CurrentInventoryId();

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastThrowAction(FVector LaunchVelocity);
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastDoMeleeAttack(int AttackIdx);

	UInventoryComponent* InventoryComp;

	FWeaponRuntimeData CurrentWeaponData;

	bool bIsInitialized = false;

	UPROPERTY()
	ATrajectoryPreview* TrajectoryPreviewRef;
	UPROPERTY()
	FTimerHandle ThrowProjectileTimer;
	
	float ThrowAngle = 10.f;
	float GrenadeInitSpeed = 1400.f;

	int32 LongGunInventoryId = FGameConstants::INVENTORY_ID_NONE;
	int32 SideArmInventoryId = FGameConstants::INVENTORY_ID_NONE; // Pistol
	int32 MeleeInventoryId = FGameConstants::INVENTORY_ID_NONE; // Knife

	ABaseCharacter* Character;
	void InitState();
public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void EquipWeapon(int32 InventoryId);
	void OnNewItemPickup(int32 NewInventoryId);
	EWeaponTypes GetCurrentWeaponType();
	void DropWeapon();
	UFUNCTION(Server, Reliable)
	void ServerOnFire(FVector TargetPoint);
	UFUNCTION(Server, Reliable)
	void ServerDoMeleeAttack(int AttackIdx);

	void OnLeftClickStart();
	void OnLeftClickRelease();
	void OnFire();
	void HandleOnFire(FVector TargetPoint);
	void StartAiming();
	void StartReload();
	bool CanShoot();
	bool IsLocalControl();

	UFUNCTION(Server, Reliable) void ServerThrow(FVector LaunchVelocity);

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlayFireRifle(FVector TargetPoint);

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastDropWeapon(int32 OnMapId, FVector DropPoint);

	bool IsScopeEquipped();

	UFUNCTION(Server, Reliable)
	void ServerDropWeapon();

	void HandleDropWeapon();

	UFUNCTION(Server, Reliable)
	void ServerEquipWeapon(int32 InventoryId);
	void OnUpdateCurrentWeaponData();
	void EquipSlot(int32 SlotIndex);

	UFUNCTION()
	void DrawProjectileCurve();

	UFUNCTION()
	void UpdateProjectileCurve();

	UFUNCTION()
	FVector GetVelocityGrenade() const;

	int GetLongGunInvenId();
	int GetSideArmInvenId();
	int GetMeleeInvenId();
	void UpdateAttachLocationWeapon();
	bool CanWeaponAim();
	void OnFinishedThrow();
	void HandleEquipWeapon(int32 InventoryId);
	void PerformMeleeAttack(int AttackIdx);
};
