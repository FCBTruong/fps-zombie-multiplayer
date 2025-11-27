// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Items/ItemData.h"
#include "MyPlayerState.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnUpdateMoney)
DECLARE_MULTICAST_DELEGATE(FOnUpdateBoughtItems)
/**
 * 
 */
UCLASS()
class FPSDEMO_API AMyPlayerState : public APlayerState
{
	GENERATED_BODY()

protected:
	FName TeamID = "";
	int PlayerID = -1;
	bool bIsAlive = true;

	UPROPERTY(ReplicatedUsing = OnRep_Money)
	int Money = 10000;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_Money();

	UPROPERTY(ReplicatedUsing = OnRep_BoughtItems)
	TArray<EItemId> BoughtItems;

	UFUNCTION()
	void OnRep_BoughtItems();
public:
	AMyPlayerState();

	FName GetTeamID() const { return TeamID; }
	int GetPlayerID() const { return PlayerID; }
	bool IsAlive() const { return bIsAlive; }
	void SetTeamID(FName NewTeamID) { TeamID = NewTeamID; }
	void SetPlayerID(int NewPlayerID) { PlayerID = NewPlayerID; }
	void SetIsAlive(bool bNewIsAlive) { bIsAlive = bNewIsAlive; }
	void ProcessBuy(const UItemData* Item);
	int GetMoney() const { return Money; }
	void ResetBoughtItems() { BoughtItems.Empty(); }

	FOnUpdateMoney OnUpdateMoney;
	FOnUpdateBoughtItems OnUpdateBoughtItems;

	bool CanBuyThisItem(const UItemData* Item) const;
};
