// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PacketListener.h"

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
    void RegisterListener(IPacketListener* Listener);
    void DeregisterListener(IPacketListener* Listener);
private:
    UNetworkManager* Owner;

    void HandleLoginReply(const std::string& Payload);

    TArray<IPacketListener*> Listeners;
};
