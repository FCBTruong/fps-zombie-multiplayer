// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Game/Characters/Components/RoleGatedComponent.h"
#include "SpikeComponent.generated.h"

class UInventoryComponent;
class UActionStateComponent;
class UEquipComponent;

enum class ESpikeActionState : uint8 {
	None,
	Planting,
	Defusing
};;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnNotifyToastMessage, const FText&);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnUpdatePlantSpikeState, bool);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnUpdateDefuseSpikeState, bool);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FPSDEMO_API USpikeComponent : public URoleGatedComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USpikeComponent();

	virtual void BeginPlay() override;
public:
	void RequestPlantSpike();
	void RequestStopPlantSpike();
	void RequestStartDefuseSpike();
	void RequestStopDefuseSpike();
	void OnDefuseSucceed();
	void OnOwnerDead();
	FOnNotifyToastMessage OnNotifyToastMessage;
	FOnUpdatePlantSpikeState OnUpdatePlantSpikeState;
	FOnUpdateDefuseSpikeState OnUpdateDefuseSpikeState;
private:
	UPROPERTY(Transient) TObjectPtr<UInventoryComponent> InventoryComp = nullptr;
	UPROPERTY(Transient) TObjectPtr<UActionStateComponent> ActionStateComp = nullptr;
	UPROPERTY(Transient) TObjectPtr<UEquipComponent> EquipComp = nullptr;
	FTimerHandle PlantTimerHandle;

	UFUNCTION(Server, Reliable)
	void ServerStartPlantSpike();

	UFUNCTION(Server, Reliable)
	void ServerStopPlantSpike();

	void FinishPlantSpike();

	bool CanPlantHere() const;
	void StartPlant_Internal();
	void StopPlant_Internal();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastStartPlantSpike();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastStopPlantSpike();

	// Defuse
	void StartDefuse_Internal();
	void StopDefuse_Internal();

	UFUNCTION(Server, Reliable)
	void ServerStartDefuseSpike();
	UFUNCTION(Server, Reliable)
	void ServerStopDefuseSpike();
	UFUNCTION(NetMulticast, Reliable)
	void MulticastStartDefuseSpike();
	UFUNCTION(NetMulticast, Reliable)
	void MulticastStopDefuseSpike();

	void LockMovement();
	void UnlockMovement();

	bool bCachedJumpAllowed;
	ESpikeActionState CurrentActionState = ESpikeActionState::None;
};
