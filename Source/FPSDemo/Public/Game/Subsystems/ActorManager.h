// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Game/Items/Pickup/PickupData.h"
#include "ActorManager.generated.h"

class ATriggerBox;
class ATargetPoint;
class APlayerStart;
class APickupItem;
class ABaseCharacter;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnNewPickupItemSpawned, APickupItem*);

UCLASS()
class FPSDEMO_API AActorManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AActorManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditInstanceOnly, Category = "Setup")
	AActor* GlobalCamera;

	UPROPERTY(EditInstanceOnly, Category = "Setup")
	AActor* BombAreaA;

	UPROPERTY(EditInstanceOnly, Category = "Setup")
	AActor* BombAreaB;

	UPROPERTY(EditInstanceOnly, Category = "Setup")
	ATriggerBox* TriggerBoxAreaA;

	UPROPERTY(EditInstanceOnly, Category = "Setup")
	ATriggerBox* TriggerBoxAreaB;

	UPROPERTY(EditInstanceOnly, Category = "Setup")
	ATargetPoint* TargetPointSpike;

	UPROPERTY(EditInstanceOnly, Category = "Setup")
	TArray<APlayerStart*> AttackerStarts;

	UPROPERTY(EditInstanceOnly, Category = "Setup")
	TArray<APlayerStart*> DefenderStarts;

	UPROPERTY(EditInstanceOnly, Category = "Setup")
	TArray<ATargetPoint*> HoldPointsA;

	UPROPERTY(EditInstanceOnly, Category = "Setup")
	TArray<ATargetPoint*> HoldPointsB;

	TSet<APlayerStart*> StartUsage;

	APlayerStart* GetRandomStart(const TArray<APlayerStart*>& Starts);
public:	
	ATriggerBox* GetAreaBombA() const { return TriggerBoxAreaA; };
	ATriggerBox* GetAreaBombB() const { return TriggerBoxAreaB; };
	FVector GetSpikeStartLocation() const;
	APlayerStart* GetRandomAttackerStart();
	APlayerStart* GetRandomDefenderStart();
	FVector RandomLocationOnMap() const;
	void ResetPlayerStartsUsage();

	UPROPERTY(EditInstanceOnly, Category = "Setup")
	AActor* MainPlane;

	static AActorManager* Get(UObject* WorldContextObject);
	FVector GetRandomHoldLocationNearBombSite(FName BombSiteName) const;
	FVector GetRandomScoutLocation() const;
	FVector DefenderWeaponInitPos;
	FVector AttackerWeaponInitPos;

	AActor* GetGlobalCamera() const {
		return GlobalCamera;
	}

	void FindAndDestroyItemNode(int32 ItemOnMapId);
	int32 GetNextItemOnMapId();
	APickupItem* CreatePickupActor(FPickupData Data);
	void CleanPickupItemsOnMap();
	APickupItem* GetPickupNode(int PickupId);
	APickupItem* GetPickupSpike() const;
	void RegisterPlayer(ABaseCharacter* Pawn);
	void UnregisterPlayer(ABaseCharacter* Pawn);
	TArray<ABaseCharacter*> GetRegisteredPlayers() const { return RegisteredPlayers; }
	void ClearRegisteredPlayers() { RegisteredPlayers.Empty(); }
	void SetPickupSpike(APickupItem* SpikeItem);

	FOnNewPickupItemSpawned OnNewPickupItemSpawned;
private:
	int CurrentPickupId;
	TMap<int32, APickupItem*> PickupItemsOnMap;
	TArray<ABaseCharacter*> RegisteredPlayers; // for clients access
	TWeakObjectPtr<APickupItem> PickupSpike;
};
