// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/MatchMode.h"
#include "GameConstants.h"

struct PlayerRoomInfo
{
    FString PlayerName;
    int32 PlayerId = FGameConstants::EMPTY_PLAYER_ID;
	bool bIsBot = false;
};

/**
 * 
 */
struct FRoomData
{
	bool bIsActive = false;
	EMatchMode Mode = EMatchMode::Spike;
	bool bHasStarted = false;
	int32 OwnerId = -1; 
	TArray<PlayerRoomInfo> Players;
	bool bIsSelfHost = true;
};