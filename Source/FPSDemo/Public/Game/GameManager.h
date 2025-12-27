// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Pickup/PickupData.h"
#include "GameManager.generated.h"

class UGlobalDataAsset;
class APickupItem;
class UCharacterAsset;
class ABaseCharacter;

/**
 * 
 */
UCLASS()
class FPSDEMO_API UGameManager : public UGameInstance
{
	GENERATED_BODY()

private:
	TMap<int32, APickupItem*> PickupItemsOnMap;
	TArray<ABaseCharacter*> RegisteredPlayers; // for clients access

protected:
	virtual void Init() override;
public:
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UGlobalDataAsset> GlobalData = nullptr;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UCharacterAsset> CharacterAsset = nullptr;

	FPickupData GetDataPickupItem(int32 ItemOnMapId);
	void FindAndDestroyItemNode(int32 ItemOnMapId);
	int32 GetNextItemOnMapId();
	APickupItem* CreatePickupActor(FPickupData Data);
	void CleanPickupItemsOnMap();
	APickupItem* GetPickupNode(int PickupId);
	APickupItem* GetPickupSpike();
	void RegisterPlayer(ABaseCharacter* Pawn);
	void UnregisterPlayer(ABaseCharacter* Pawn);
	TArray<ABaseCharacter*> GetRegisteredPlayers() const { return RegisteredPlayers; }
	void ClearRegisteredPlayers() { RegisteredPlayers.Empty(); }
	TWeakObjectPtr<APickupItem> PickupSpike;

	int CurrentPickupId;
	static UGameManager* Get(UObject* WorldContextObject);
};
