// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Items/ItemIds.h"
#include "Characters/CharacterRole.h"
#include "Data/TeamId.h"
#include "MyPlayerState.generated.h"

class UItemConfig;
class APlayerSlot;

DECLARE_MULTICAST_DELEGATE(FOnUpdateMoney);
DECLARE_MULTICAST_DELEGATE(FOnUpdateBoughtItems);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnUpdateTeamId, ETeamId);
/**
 * 
 */
UCLASS()
class FPSDEMO_API AMyPlayerState : public APlayerState
{
	GENERATED_BODY()

protected:
	UPROPERTY(ReplicatedUsing = OnRep_Money)
	int Money = 10000;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_Money();

	UPROPERTY(ReplicatedUsing = OnRep_BoughtItems)
	TArray<EItemId> BoughtItems;

	UFUNCTION()
	void OnRep_BoughtItems();

	TArray<EItemId> OwnedWeapons;

	UPROPERTY(Replicated)
	APlayerSlot* PlayerSlot = nullptr;

	bool bWasChosenAsZombie = false;
public:
	AMyPlayerState();
	virtual void PostInitializeComponents() override;

	ETeamId GetTeamId() const;
	void SetTeamId(ETeamId NewTeamId);
	void ProcessBuy(const UItemConfig* Item);
	int GetMoney() const { return Money; }
	void ResetBoughtItems() { BoughtItems.Empty(); }

	FOnUpdateMoney OnUpdateMoney;
	FOnUpdateBoughtItems OnUpdateBoughtItems;
	FOnUpdateTeamId OnUpdateTeamId;

	bool CanBuyThisItem(const UItemConfig* Item) const;
	void AutoBuy();
	void TryBuySlot();
	void AddOwnedWeapon(EItemId Id) {
		OwnedWeapons.Add(Id);
	}
	void ClearOwnedWeapons() {
		OwnedWeapons.Empty();
	}
	TArray<EItemId> GetOwnedWeapons() const {
		return OwnedWeapons;
	}

	void AddKill();
	void AddDeath();
	void AddAssist();
	int GetKills() const;
	int GetDeaths() const;
	int GetAssists() const;

	void SetChosenAsZombie(bool bChosen) {
		bWasChosenAsZombie = bChosen;
	}
	bool WasChosenAsZombie() const {
		return bWasChosenAsZombie;
	}

	int GetBackendUserId() const;

	void SetPlayerSlot(APlayerSlot* Slot) {
		PlayerSlot = Slot;
	}

	APlayerSlot* GetPlayerSlot() const {
		return PlayerSlot;
	}
};
