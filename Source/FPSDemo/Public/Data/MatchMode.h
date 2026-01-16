#pragma once

UENUM(BlueprintType)
enum class EMatchMode : uint8
{
	None = 0,
	Spike = 1,
	Zombie = 2,
	DeathMatch = 3
};