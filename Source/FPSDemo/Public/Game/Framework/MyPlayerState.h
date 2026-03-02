// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Shared/Types/ItemId.h"
#include "Game/Characters/CharacterRole.h"
#include "Game/Types/TeamId.h"
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

public:
	AMyPlayerState();

	ETeamId GetTeamId() const;
	void SetTeamId(ETeamId NewTeamId);
	void ProcessBuy(const UItemConfig* Item);
	void ResetBoughtItems() { BoughtItems.Empty(); }
	bool CanBuyThisItem(const UItemConfig* Item) const;
	void AutoBuy();
	void TryBuySlot();
	void AddOwnedWeapon(EItemId Id) {
		OwnedWeapons.Add(Id);
	}
	void ClearOwnedWeapons() {
		OwnedWeapons.Empty();
	}
	void AddKill();
	void AddDeath();
	void AddAssist();

	int GetKills() const;
	int GetDeaths() const;
	int GetAssists() const;
	int GetMoney() const { return Money; }
	int GetBackendUserId() const;

	TArray<EItemId> GetOwnedWeapons() const {
		return OwnedWeapons;
	}

	void SetChosenAsZombie(bool bChosen) {
		bWasChosenAsZombie = bChosen;
	}
	bool WasChosenAsZombie() const {
		return bWasChosenAsZombie;
	}
	void SetPlayerSlot(APlayerSlot* Slot);
	APlayerSlot* GetPlayerSlot() const {
		return PlayerSlot;
	}
	virtual FString GetPlayerNameCustom() const override;
	void SetCrosshairCode(const FString& InCrosshairCode);
	const FString& GetCrosshairCode() const {
		return CrosshairCode;
	}

public:
	// delegates
	FOnUpdateMoney OnUpdateMoney;
	FOnUpdateBoughtItems OnUpdateBoughtItems;
	FOnUpdateTeamId OnUpdateTeamId;

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UPROPERTY(ReplicatedUsing = OnRep_Money)
	int32 Money;

	UPROPERTY(Replicated)
	FString CrosshairCode;

	UPROPERTY(ReplicatedUsing = OnRep_BoughtItems)
	TArray<EItemId> BoughtItems;

	UFUNCTION()
	void OnRep_BoughtItems();

	UFUNCTION()
	void OnRep_PlayerSlot();

	UFUNCTION()
	void OnRep_Money();
	void HandlePlayerSlotTeamIdUpdated(ETeamId NewTeamId);

	UPROPERTY(ReplicatedUsing = OnRep_PlayerSlot)
	APlayerSlot* PlayerSlot = nullptr;

	bool bWasChosenAsZombie = false;
	TArray<EItemId> OwnedWeapons;
};
