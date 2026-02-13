// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Shared/Types/MatchMode.h"

struct FPlayerMatchInfo
{
	FString PlayerName;
	int32 PlayerId;
	bool bIsBot = false;
	FString Avatar;
	FString CrosshairCode;
};

struct FMatchInfo
{
	int32 RoomId = -1;
	EMatchMode Mode = EMatchMode::Spike;
	TArray<FPlayerMatchInfo> Players;
	bool bIsSelfHost = true;
};
