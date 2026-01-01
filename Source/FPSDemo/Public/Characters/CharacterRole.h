
#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
enum class ECharacterRole : uint8
{
	Human	UMETA(DisplayName = "Human"),
	Zombie	UMETA(DisplayName = "Zombie"),
	Hero	UMETA(DisplayName = "Hero")
};

