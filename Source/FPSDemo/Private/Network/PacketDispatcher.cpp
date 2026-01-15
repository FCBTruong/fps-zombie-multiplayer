// Fill out your copyright notice in the Description page of Project Settings.


#include "Network/PacketDispatcher.h"
#include "Network/NetworkManager.h"

FPacketDispatcher::FPacketDispatcher(UNetworkManager* InOwner)
    : Owner(InOwner)
{
}

void FPacketDispatcher::Dispatch(const game::net::Packet& Packet)
{
    switch (static_cast<ECmdId>(Packet.cmd_id()))
    {
    case ECmdId::LOGIN_REPLY:
        HandleLoginReply(Packet.payload());
        break;
    case ECmdId::CREATE_ROOM:
        HandleCreateRoom(Packet.payload());
        break;
    default:
        UE_LOG(LogTemp, Warning,
            TEXT("Unhandled CmdId: %d"),
            Packet.cmd_id());
        break;
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
    Owner->HandleLoginSuccess(Token);
}   

void FPacketDispatcher::HandleCreateRoom(const std::string& Payload)
{
    game::net::CreateRoom Msg;
    if (!Msg.ParseFromString(Payload))
        return;

    Owner->HandleCreateRoom();
}

