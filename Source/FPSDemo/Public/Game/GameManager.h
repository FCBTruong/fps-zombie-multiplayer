// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Pickup/PickupData.h"
#include "Network/DedicatedServerClient.h"
#include "GameManager.generated.h"

class UGlobalDataAsset;
class APickupItem;
class UCharacterAsset;
class ABaseCharacter;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnNewPickupItemSpawned, APickupItem*);
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
	TWeakObjectPtr<APickupItem> PickupSpike;
protected:
	virtual void Init() override;
	virtual void OnStart() override;
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
	APickupItem* GetPickupSpike() const;
	void RegisterPlayer(ABaseCharacter* Pawn);
	void UnregisterPlayer(ABaseCharacter* Pawn);
	TArray<ABaseCharacter*> GetRegisteredPlayers() const { return RegisteredPlayers; }
	void ClearRegisteredPlayers() { RegisteredPlayers.Empty(); }
	void SetPickupSpike(APickupItem* SpikeItem);
	int CurrentPickupId;
	static UGameManager* Get(UObject* WorldContextObject);
	TUniquePtr<DedicatedServerClient> DsClient;

	void StartMatch();
	void InitServerConfig(
		const FString& InRoomId,
		const FString& InToken);
	void RequestMatchDataAndStart();
	void OnWorldCleanup(
		UWorld* World,
		bool bSessionEnded,
		bool bCleanupResources);
	FOnNewPickupItemSpawned OnNewPickupItemSpawned;

private:
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void CreateHostSession();
	FName PendingMapName;
	FString PendingOptions;
	FDelegateHandle OnCreateHandle;
};
