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

	static constexpr int32 INIT_HEALTH_SOLIDER = 500;
	static constexpr int32 INIT_HEALTH_ZOMBIE = 500;
	static constexpr int32 INIT_HEALTH_HERO = 600;

	static constexpr int32 EMPTY_PLAYER_ID = -1;
	static constexpr int32 BOT_PLAYER_ID_START = 100000;

	static constexpr int32 AVATAR_IDS[] = { 14, 15, 16, 17, 18, 20, 21 };

    static const FName LEVEL_LOADING_SCREEN;
    static const FName LEVEL_PLAYGROUND;
    static const FName LEVEL_LOBBY;
    static const FName LEVEL_GHOST_MALL_MAP;
    static const FName LEVEL_PRACTICE;
    static const FName LEVEL_LOADING;
};
