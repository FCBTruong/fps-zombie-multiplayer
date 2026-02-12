// Fill out your copyright notice in the Description page of Project Settings.


#include "Modules/Chat/ChatSubsystem.h"
#include "Network/NetworkManager.h"


void UChatSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);
    Collection.InitializeDependency<UNetworkManager>();

    NetworkManager =
        Collection.InitializeDependency<UNetworkManager>();

    check(NetworkManager);

    NetworkManager->RegisterListener(this);
}

void UChatSubsystem::OnPacketReceived(
    ECmdId CmdId,
    const std::string& Payload
)
{
    // Handle chat-related packets here
    switch (CmdId)
    {
    case ECmdId::CHAT_IN_ROOM:
    {
        HandleChatMessage(Payload);
        break;
    }
    default:
		break;
    }
}

void UChatSubsystem::HandleChatMessage(const std::string& payload)
{
    // Handle chat message in room
    game::net::ChatInRoomReply Msg;
    if (!Msg.ParseFromString(payload))
        return;
    FString Sender = FString(Msg.sender().c_str());
    FString Message = FString(Msg.mess().c_str());

    UE_LOG(LogTemp, Log, TEXT("Chat Message from %s: %s"), *Sender, *Message);

	OnNewChatMessage.Broadcast(Sender, Message);
}

void UChatSubsystem::SendChatMessageInRoom(const FString& Message)
{
    if (!NetworkManager)
        return;

    game::net::ChatInRoomRequest Msg;
    Msg.set_mess(TCHAR_TO_UTF8(*Message));
    NetworkManager->SendPacket(ECmdId::CHAT_IN_ROOM, Msg);
}