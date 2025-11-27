// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Pickup/PickupData.h"
#include "Items/ItemData.h"
#include "Game/GlobalDataAsset.h"
#include "Weapons/WeaponDataManager.h"
#include "GameManager.generated.h"

/**
 * 
 */
UCLASS()
class FPSDEMO_API UGameManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

private:
	UPROPERTY()
	TMap<int32, AActor*> ItemNodesOnMap;
	UWeaponDataManager* WeaponDataManager;
public:
	void GenItemNodesOnMap(const TArray<FPickupData>& Items);
	void OnReceivedItemsFromServer(const TArray<FPickupData>& Items);
	FPickupData GetDataPickupItem(int32 ItemOnMapId);
	void FindAndDestroyItem(int32 ItemOnMapId);
	UItemData* GetItemDataById(EItemId ItemId);
	void OnNewItemDataSpawned(const TArray<FPickupData>& Items);
	void OnNewItemNodeSpawned(AActor* Item, int32 OnMapId);
	int32 GetNextItemOnMapId();
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	UWeaponDataManager* GetWeaponDataManager();

	UGlobalDataAsset* GlobalData;
};
