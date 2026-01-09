// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Items/ItemIds.h"
#include "Characters/CharacterRole.h"
#include "MyPlayerState.generated.h"

class UItemConfig;

DECLARE_MULTICAST_DELEGATE(FOnUpdateMoney);
DECLARE_MULTICAST_DELEGATE(FOnUpdateBoughtItems);
/**
 * 
 */
UCLASS()
class FPSDEMO_API AMyPlayerState : public APlayerState
{
	GENERATED_BODY()

protected:
	UPROPERTY(ReplicatedUsing = OnRep_TeamId)
	FName TeamID = "";

	UPROPERTY(ReplicatedUsing = OnRep_Money)
	int Money = 10000;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_Money();

	UFUNCTION()
	void OnRep_TeamId();

	UPROPERTY(ReplicatedUsing = OnRep_BoughtItems)
	TArray<EItemId> BoughtItems;

	UFUNCTION()
	void OnRep_BoughtItems();

	TArray<EItemId> OwnedWeapons;

	UPROPERTY(Replicated)
	int Kills = 0;

	UPROPERTY(Replicated)
	int Deaths = 0;

	UPROPERTY(Replicated)
	int Assists = 0;
public:
	AMyPlayerState();

	FName GetTeamID() const { return TeamID; }
	void SetTeamID(FName NewTeamID) { TeamID = NewTeamID; }
	void ProcessBuy(const UItemConfig* Item);
	int GetMoney() const { return Money; }
	void ResetBoughtItems() { BoughtItems.Empty(); }

	FOnUpdateMoney OnUpdateMoney;
	FOnUpdateBoughtItems OnUpdateBoughtItems;

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

	void AddKill() {
		Kills++;
	}
	void AddDeath() {
		Deaths++;
	}
	void AddAssist() {
		Assists++;
	}
	int GetKills() const {
		return Kills;
	}
	int GetDeaths() const {
		return Deaths;
	}
	int GetAssists() const {
		return Assists;
	}
};
