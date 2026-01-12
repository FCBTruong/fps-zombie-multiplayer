// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/MatchMode.h"

struct PlayerRoomInfo
{
    FString PlayerName;
    int32 PlayerId = -1;    
	bool bIsBot = false;
};

/**
 * 
 */
struct RoomData
{
	EMatchMode Mode = EMatchMode::Spike;
	bool bHasStarted = false;
	int32 OwnerId = -1; 
	TArray<PlayerRoomInfo> Players;
	bool bIsSelfHost = true;
};