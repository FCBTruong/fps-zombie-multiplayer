// Fill out your copyright notice in the Description page of Project Settings.


#include "Network/PacketDispatcher.h"
#include "Network/NetworkManager.h"

FPacketDispatcher::FPacketDispatcher(UNetworkManager* InOwner)
    : Owner(InOwner)
{
}

void FPacketDispatcher::Dispatch(const game::net::Packet& Packet)
{
	ECmdId CmdId = static_cast<ECmdId>(Packet.cmd_id());

    UE_LOG(LogTemp, Log, TEXT("Received: CmdId = %d"), static_cast<int32>(CmdId));

	const std::string& Payload = Packet.payload();
    switch (CmdId)
    {
    case ECmdId::LOGIN_REPLY:
        HandleLoginReply(Packet.payload());
        break;
    default:
        break;
    }

    for (IPacketListener* Listener : Listeners)
    {
        Listener->OnPacketReceived(CmdId, Payload);
    }
}

void FPacketDispatcher::HandleLoginReply(const std::string& Payload)
{
    game::net::LoginReply Reply;
    if (!Reply.ParseFromString(Payload))
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to parse LoginReply"));
        return;
    }

    const FString Token = UTF8_TO_TCHAR(Reply.token().c_str());

    UE_LOG(LogTemp, Log,
        TEXT("Login successful. UserId=%d Token=%s"),
        Reply.user_id(),
		*Token);

    // Call back into NetworkManager
    Owner->HandleLoginSuccess(Reply);
}   

void FPacketDispatcher::RegisterListener(IPacketListener* Listener)
{
    if (!Listener)
        return;

    UE_LOG(
        LogTemp,
        Log,
        TEXT("FPacketDispatcher: Registering listener")
    );

    Listeners.AddUnique(Listener);
}

void FPacketDispatcher::DeregisterListener(IPacketListener* Listener)
{
    Listeners.Remove(Listener);
}