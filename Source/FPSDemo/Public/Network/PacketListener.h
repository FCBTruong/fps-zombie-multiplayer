// IPacketListener.h
#pragma once

#include "game.pb.h"
#include "Network/CmdId.h"

class IPacketListener
{
public:
    virtual ~IPacketListener() = default;

    virtual void OnPacketReceived(
        ECmdId CmdId,
        const std::string& Payload
    ) = 0;
};