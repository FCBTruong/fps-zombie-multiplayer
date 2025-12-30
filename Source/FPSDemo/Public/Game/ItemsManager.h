// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Items/ItemIds.h"
#include "ItemsManager.generated.h"

class UItemConfig;
/**
 * 
 */
UCLASS()
class FPSDEMO_API UItemsManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

private:
	UPROPERTY()
	TArray<UItemConfig*> ItemList;
	UPROPERTY()
	TMap<EItemId, UItemConfig*> ItemConfigMap;
public:
	const UItemConfig* GetItemById(EItemId Id) const;

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	const TArray<UItemConfig*>& GetAllItems() const { return ItemList; }

	static UItemsManager* Get(UWorld* WorldContextObject);
};
