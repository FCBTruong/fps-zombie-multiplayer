// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class EMyMatchState : uint8
{
	PRE_MATCH,
	ROUND_START,
	BUY_PHASE,
	ROUND_IN_PROGRESS,
	SPIKE_PLANTED,
	ROUND_ENDED,
	GAME_ENDED
};
