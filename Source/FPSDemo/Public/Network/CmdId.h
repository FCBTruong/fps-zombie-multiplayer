#pragma once


UENUM(BlueprintType)
enum class ECmdId : uint8 {
    HANDSHAKE = 0,
    LOGIN = 1,
    LOGIN_REPLY = 2,
    CREATE_ROOM = 100,
    LEAVE_ROOM = 101,
    JOIN_ROOM = 102
};