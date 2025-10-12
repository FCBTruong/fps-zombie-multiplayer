// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Pickup/PickupData.h"
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
	void GenItemsOnMap(const TArray<FPickupData>& Items);
	FPickupData GetDataPickupItem(int32 ItemOnMapId);
	void FindAndDestroyItem(int32 ItemOnMapId);
};
