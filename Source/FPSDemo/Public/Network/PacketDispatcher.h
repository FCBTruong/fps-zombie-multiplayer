// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "game.pb.h"

class UNetworkManager;

/**
 * 
 */
class FPSDEMO_API FPacketDispatcher
{
public:
    explicit FPacketDispatcher(UNetworkManager* InOwner);

    // Entry point on GameThread
    void Dispatch(const game::net::Packet& Packet);

private:
    UNetworkManager* Owner;

    void HandleLoginReply(const std::string& Payload);
    void HandleCreateRoom(const std::string& Payload);
};
