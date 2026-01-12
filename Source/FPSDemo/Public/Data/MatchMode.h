#pragma once

UENUM(BlueprintType)
enum class EMatchMode : uint8
{
	None,
	Spike,
	Zombie,
	DeathMatch
};