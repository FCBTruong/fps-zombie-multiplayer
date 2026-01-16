// Fill out your copyright notice in the Description page of Project Settings.


#include "Lobby/RoomManager.h"
#include "Network/NetworkManager.h"
#include "Lobby/PlayerInfoManager.h"

void URoomManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
    Collection.InitializeDependency<UNetworkManager>();

    NetworkManager =
        Collection.InitializeDependency<UNetworkManager>();

    check(NetworkManager);

    NetworkManager->RegisterListener(this);
}

void URoomManager::OnPacketReceived(
	ECmdId CmdId,
	const std::string& Payload
)
{
	// Handle room-related packets here
    switch (CmdId)
    {
    case ECmdId::ROOM_INFO:
    {
        game::net::RoomInfoReply RoomInfoPkg;
        if (!RoomInfoPkg.ParseFromString(Payload))
            return;
        CurrentRoomData.bHasStarted = RoomInfoPkg.has_started();
        CurrentRoomData.bIsActive = true;
        CurrentRoomData.Mode = static_cast<EMatchMode>(RoomInfoPkg.mode());
        CurrentRoomData.bIsSelfHost = RoomInfoPkg.is_self_host();

        CurrentRoomData.Players.Empty();
        for (int i = 0; i < RoomInfoPkg.players_size(); ++i) {
            const auto& ProtoPlayerInfo = RoomInfoPkg.players(i);
            PlayerRoomInfo Info;
            Info.PlayerName = FString(ProtoPlayerInfo.player_name().c_str());
            Info.PlayerId = ProtoPlayerInfo.user_id();
            Info.bIsBot = ProtoPlayerInfo.is_bot();
            CurrentRoomData.Players.Add(Info);
        }

        OnRoomInfoReceived.Broadcast();
        break;
    }
    case ECmdId::LIST_ROOM:
    {
        game::net::ListRoomReply Msg;
        if (!Msg.ParseFromString(Payload))
            return;

		AvailableRooms.Empty();

        for (int i = 0; i < Msg.rooms_size(); ++i) {
            const auto& ProtoRoom = Msg.rooms(i);
            FRoomData RoomData;
            RoomData.bIsActive = true;
            RoomData.Mode = static_cast<EMatchMode>(ProtoRoom.mode());
            RoomData.bHasStarted = ProtoRoom.has_started();
            RoomData.OwnerId = ProtoRoom.owner_id();
            RoomData.Players.Empty();
            for (int j = 0; j < ProtoRoom.players_size(); ++j) {
                const auto& ProtoPlayerInfo = ProtoRoom.players(j);
                PlayerRoomInfo Info;
                Info.PlayerName = FString(ProtoPlayerInfo.player_name().c_str());
                Info.PlayerId = ProtoPlayerInfo.user_id();
                Info.bIsBot = ProtoPlayerInfo.is_bot();
                RoomData.Players.Add(Info);
            }
            AvailableRooms.Add(RoomData);
		}

		OnUpdateRoomList.Broadcast();
        break;
    }
    case ECmdId::KICK_PLAYER:
        // Handle kick player response
        break;
    case ECmdId::SWITCH_SLOT: {
        game::net::SwitchSlotReply Msg;
        if (!Msg.ParseFromString(Payload))
            return;
		int SlotIdxA = Msg.slot_idx_a();
		int SlotIdxB = Msg.slot_idx_b();
        if (CurrentRoomData.Players.IsValidIndex(SlotIdxA) &&
            CurrentRoomData.Players.IsValidIndex(SlotIdxB)) {
            // Swap the player info in the two slots
            CurrentRoomData.Players.Swap(SlotIdxA, SlotIdxB);
            OnUpdateRoomSlot.Broadcast(SlotIdxA);
			OnUpdateRoomSlot.Broadcast(SlotIdxB);
		}
        break;
    }
    case ECmdId::LEAVE_ROOM:
    {
        game::net::PlayerLeaveReply Msg;
        if (!Msg.ParseFromString(Payload))
            return;
		int SlotIdx = Msg.slot_idx();
        if (CurrentRoomData.Players.IsValidIndex(SlotIdx)) {
            // Clear the player info in that slot
            PlayerRoomInfo Info;

            // if is me, then leave room scene
            if (Info.PlayerId == UPlayerInfoManager::Get(GetWorld())->GetUserId()) {
                CurrentRoomData = FRoomData(); // reset room data
                OnRoomInfoReceived.Broadcast();
                return;
			}

            Info.PlayerName = FString(TEXT(""));
            Info.PlayerId = FGameConstants::EMPTY_PLAYER_ID; // Indicate empty slot
            Info.bIsBot = false;
            CurrentRoomData.Players[SlotIdx] = Info;
            OnUpdateRoomSlot.Broadcast(SlotIdx);
		}
        break;
    }
    case ECmdId::NEW_PLAYER_JOIN_ROOM: {
        game::net::NewPlayerJoinRoomReply Msg;
        if (!Msg.ParseFromString(Payload))
            return;

        // update current room data
		const int SlotIdx = Msg.slot_idx();
        if (CurrentRoomData.Players.IsValidIndex(SlotIdx)) {
            const auto& ProtoPlayerInfo = Msg.player_info();
            PlayerRoomInfo Info;
            Info.PlayerName = FString(ProtoPlayerInfo.player_name().c_str());
            Info.PlayerId = ProtoPlayerInfo.user_id();
            Info.bIsBot = ProtoPlayerInfo.is_bot();
            CurrentRoomData.Players[SlotIdx] = Info;

            OnUpdateRoomSlot.Broadcast(SlotIdx);
        }
		break;
    }
    default:
        break;
    }
}

URoomManager* URoomManager::Get(UWorld* World)
{
    if (!World)
    {
        return nullptr;
    }
    UGameInstance* GameInstance = World->GetGameInstance();
    if (!GameInstance)
    {
        return nullptr;
    }
    return GameInstance->GetSubsystem<URoomManager>();
}

void URoomManager::RequestKickPlayer(int PlayerId)
{
    if (NetworkManager) {
        game::net::KickPlayerRequest Pkg;
		Pkg.set_user_id(PlayerId);
        NetworkManager->SendPacket(ECmdId::KICK_PLAYER, Pkg);
    }
}

void URoomManager::RequestSwitchSlot(int SlotIdx)
{
    if (NetworkManager) {
        game::net::SwitchSlotRequest Pkg;
		Pkg.set_slot_idx(SlotIdx);
        NetworkManager->SendPacket(ECmdId::SWITCH_SLOT, Pkg);
    }
}

void URoomManager::RequestAddBot(int team) {
    if (NetworkManager) {
        game::net::AddBotRequest Pkg;
        Pkg.set_team(team);
        NetworkManager->SendPacket(ECmdId::ADD_BOT, Pkg);
    }
}

void URoomManager::RequestRoomList()
{
    if (NetworkManager) {
        game::net::Empty Pkg;
        NetworkManager->SendPacket(ECmdId::LIST_ROOM, Pkg);
    }
}