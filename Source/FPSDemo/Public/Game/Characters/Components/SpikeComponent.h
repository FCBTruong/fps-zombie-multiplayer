// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Game/Characters/Components/RoleGatedComponent.h"
#include "SpikeComponent.generated.h"

class UInventoryComponent;
class UActionStateComponent;
class UEquipComponent;
class ABaseCharacter;

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
	void Init();

public:
	void RequestPlantSpike();
	void RequestStopPlantSpike();
	void RequestStartDefuseSpike();
	void RequestStopDefuseSpike();
	void OnDefuseSucceed();
	void OnOwnerDead();

	// Delegates
	FOnNotifyToastMessage OnNotifyToastMessage;
	FOnUpdatePlantSpikeState OnUpdatePlantSpikeState;
	FOnUpdateDefuseSpikeState OnUpdateDefuseSpikeState;

	const float DefuseDistance = 200.f;
	const float PlantTime = 3.f;
private:
	UInventoryComponent* InventoryComp = nullptr;
	UActionStateComponent* ActionStateComp = nullptr;
	UEquipComponent* EquipComp = nullptr;
	ABaseCharacter* Character = nullptr;

	FTimerHandle PlantTimerHandle;

	UFUNCTION(Server, Reliable)
	void ServerStartPlantSpike();

	UFUNCTION(Server, Reliable)
	void ServerStopPlantSpike();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastStartPlantSpike();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastStopPlantSpike();

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
	// Defuse
	void StartDefuse_Internal();
	void StopDefuse_Internal();
	void FinishPlantSpike();
	void StartPlant_Internal();
	void StopPlant_Internal();
	bool CanPlantHere() const;

	bool bCachedJumpAllowed;
};
