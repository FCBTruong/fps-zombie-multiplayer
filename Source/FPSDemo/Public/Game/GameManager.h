// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Pickup/PickupData.h"
#include "Items/ItemData.h"
#include "Game/GlobalDataAsset.h"
#include "Weapons/WeaponDataManager.h"
#include "Pickup/PickupItem.h"
#include "GameManager.generated.h"

/**
 * 
 */
UCLASS()
class FPSDEMO_API UGameManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

private:
	UWeaponDataManager* WeaponDataManager;
	TMap<int32, APickupItem*> PickupItemsOnMap;
public:
	FPickupData GetDataPickupItem(int32 ItemOnMapId);
	void FindAndDestroyItemNode(int32 ItemOnMapId);
	UItemData* GetItemDataById(EItemId ItemId);
	int32 GetNextItemOnMapId();
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	UWeaponDataManager* GetWeaponDataManager();

	UGlobalDataAsset* GlobalData;
	UWeaponData* GetWeaponDataById(EItemId ItemId);
	APickupItem* CreatePickupActor(FPickupData Data);
	void CleanPickupItemsOnMap();
	APickupItem* GetPickupNode(int PickupId);

	static UGameManager* Instance;
	static int CurrentPickupId;
	APickupItem* GetPickupSpike();
};
