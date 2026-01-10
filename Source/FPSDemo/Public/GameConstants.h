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

	static constexpr int32 MELEE_ATTACK_INDEX_PRIMARY = 1;
	static constexpr int32 MELEE_ATTACK_INDEX_SECONDARY = 2;

    static constexpr int32 ZOMBIE_TEAM_ID = 1;
	static constexpr int32 SODIER_TEAM_ID = 2;

	static constexpr int32 INIT_HEALTH_SOLIDER = 100;
	static constexpr int32 INIT_HEALTH_ZOMBIE = 500;
	static constexpr int32 INIT_HEALTH_HERO = 600;
};
