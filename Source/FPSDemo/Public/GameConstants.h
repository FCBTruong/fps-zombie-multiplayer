#pragma once
#include "CoreMinimal.h"

class FPSDEMO_API FGameConstants
{
public:
    static constexpr int32 INVENTORY_ID_NONE = -1;

    static constexpr int32 SLOT_NONE = 0;
    static constexpr int32 SLOT_RIFLE = 1;
    static constexpr int32 SLOT_PISTOL = 2;
    static constexpr int32 SLOT_MELEE = 3;
    static constexpr int32 SLOT_THROWABLE = 4;
	static constexpr int32 SLOT_SPIKE = 5;
};
