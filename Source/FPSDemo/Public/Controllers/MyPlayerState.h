// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Items/ItemData.h"
#include "MyPlayerState.generated.h"

/**
 * 
 */
UCLASS()
class FPSDEMO_API AMyPlayerState : public APlayerState
{
	GENERATED_BODY()

private:
	FName TeamID = "";
	int PlayerID = -1;
	bool bIsAlive = true;
	int Money = 1000;


public:
	AMyPlayerState();

	FName GetTeamID() const { return TeamID; }
	int GetPlayerID() const { return PlayerID; }
	bool IsAlive() const { return bIsAlive; }
	void SetTeamID(FName NewTeamID) { TeamID = NewTeamID; }
	void SetPlayerID(int NewPlayerID) { PlayerID = NewPlayerID; }
	void SetIsAlive(bool bNewIsAlive) { bIsAlive = bNewIsAlive; }
	void ProcessBuy(const UItemData* Item);
};
