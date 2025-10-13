// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Pickup/PickupData.h"
#include "Items/ItemData.h"
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
	TMap<int32, AActor*> ItemsOnMap;
public:
	void GenItemNodesOnMap(const TArray<FPickupData>& Items);
	void OnReceivedItemsFromServer(const TArray<FPickupData>& Items);
	FPickupData GetDataPickupItem(int32 ItemOnMapId);
	void FindAndDestroyItem(int32 ItemOnMapId);
	UItemData* GetItemDataById(EItemId ItemId);
};
