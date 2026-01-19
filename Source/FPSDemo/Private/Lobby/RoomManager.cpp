// Fill out your copyright notice in the Description page of Project Settings.


#include "Lobby/RoomManager.h"
#include "Network/NetworkManager.h"
#include "Lobby/PlayerInfoManager.h"
#include "Network/NetBoostrapSubsystem.h"
#include "Kismet/GameplayStatics.h"

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
		CurrentRoomData.OwnerId = RoomInfoPkg.owner_id();
		CurrentRoomData.RoomId = RoomInfoPkg.room_id();
        CurrentRoomData.Mode = static_cast<EMatchMode>(RoomInfoPkg.mode());
        CurrentRoomData.bIsSelfHost = RoomInfoPkg.is_self_host();
		CurrentRoomData.JoinKey = FString(RoomInfoPkg.join_key().c_str());

        CurrentRoomData.Players.Empty();    
        for (int i = 0; i < RoomInfoPkg.players_size(); ++i) {
            const auto& ProtoPlayerInfo = RoomInfoPkg.players(i);
            PlayerRoomInfo Info;
            Info.PlayerName = FString(ProtoPlayerInfo.player_name().c_str());
            Info.PlayerId = ProtoPlayerInfo.user_id();
            Info.bIsBot = ProtoPlayerInfo.is_bot();
			Info.Avatar = FString(ProtoPlayerInfo.avatar().c_str());
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
			RoomData.RoomId = ProtoRoom.room_id();
            RoomData.Players.Empty();
            for (int j = 0; j < ProtoRoom.players_size(); ++j) {
                const auto& ProtoPlayerInfo = ProtoRoom.players(j);
                PlayerRoomInfo Info;
                Info.PlayerName = FString(ProtoPlayerInfo.player_name().c_str());
                Info.PlayerId = ProtoPlayerInfo.user_id();
                Info.bIsBot = ProtoPlayerInfo.is_bot();
				Info.Avatar = FString(ProtoPlayerInfo.avatar().c_str());
                RoomData.Players.Add(Info);
            }
            AvailableRooms.Add(RoomData);
		}

		OnUpdateRoomList.Broadcast();
        break;
    }
    case ECmdId::KICK_PLAYER:
    {   // Handle kick player response
        break;
    }
    case ECmdId::SWITCH_SLOT: {
        UE_LOG(LogTemp, Warning, TEXT("debug::switch"));
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
    case ECmdId::LEAVE_ROOM: {
		CurrentRoomData = FRoomData(); // reset room data
		OnRoomInfoReceived.Broadcast();
        break;
    }
    case ECmdId::PLAYER_LEAVE_ROOM_NOTI:
    {
        UE_LOG(LogTemp, Warning, TEXT("debug::I have left the room"));
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
			Info.Avatar = FString(ProtoPlayerInfo.avatar().c_str());
            CurrentRoomData.Players[SlotIdx] = Info;

            OnUpdateRoomSlot.Broadcast(SlotIdx);
        }
		break;
    }
    case ECmdId::ROOM_UPDATE_GAME_MODE: {
		game::net::ChangeGameModeReply Msg;
		if (!Msg.ParseFromString(Payload))
			return;

		CurrentRoomData.Mode = static_cast<EMatchMode>(Msg.game_mode());
		OnUpdateRoomGameMode.Broadcast();
        break;
    }
    case ECmdId::ROOM_UPDATE_HOST_TYPE: {
		game::net::ChangeHostTypeReply Msg;
		if (!Msg.ParseFromString(Payload))
			return;
		CurrentRoomData.bIsSelfHost = Msg.is_self_host();
		OnUpdateRoomHostType.Broadcast();
        break;
    }
    case ECmdId::ROOM_UPDATE_OWNER: {
		game::net::RoomUpdateOwnerReply Msg;
        if (!Msg.ParseFromString(Payload))
			return;
		CurrentRoomData.OwnerId = Msg.new_owner_id();
		OnUpdateRoomOwner.Broadcast();
        break;
    }
    case ECmdId::GAME_STARTED:
    {
        HandleGameStarted(Payload);
        break;
    }
    case ECmdId::SELFHOST_READY:
    {
		HandleSelfHostReady(Payload);
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
    if (NetworkManager && NetworkManager->IsConnected()) {
        game::net::KickPlayerRequest Pkg;
		Pkg.set_user_id(PlayerId);
        NetworkManager->SendPacket(ECmdId::KICK_PLAYER, Pkg);
    }
    else {
		// offline mode
        for (int i = 0; i < CurrentRoomData.Players.Num(); ++i)
        {
            if (CurrentRoomData.Players[i].PlayerId == PlayerId)
            {
                PlayerRoomInfo Info;
                Info.PlayerName = TEXT("");
                Info.PlayerId = FGameConstants::EMPTY_PLAYER_ID; // Indicate empty slot
                Info.bIsBot = false;
                CurrentRoomData.Players[i] = Info;
                OnUpdateRoomSlot.Broadcast(i);
                break;
            }
        }
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
    if (NetworkManager && NetworkManager->IsConnected()) {
        game::net::AddBotRequest Pkg;
        Pkg.set_team(team);
        NetworkManager->SendPacket(ECmdId::ADD_BOT, Pkg);
    }
    else {
		// offline mode

        // find empty slot
		int StartIdx = 0;
        if (team == URoomManager::TEAM_2) {
            StartIdx = 5;
        }
        for (int i = StartIdx; i < StartIdx + 5; ++i)
        {
            if (CurrentRoomData.Players[i].PlayerId == FGameConstants::EMPTY_PLAYER_ID)
            {
                PlayerRoomInfo Info;
				Info.PlayerName = TEXT("Bot");
				Info.Avatar = "100"; // default bot avatar
                Info.PlayerId = FGameConstants::BOT_PLAYER_ID_START + i; // assign a bot id
                Info.bIsBot = true;
                CurrentRoomData.Players[i] = Info;
                OnUpdateRoomSlot.Broadcast(i);
                break;
            }
		}
    }
}

void URoomManager::RequestRoomList()
{
    if (NetworkManager && NetworkManager->IsConnected()) {
        game::net::Empty Pkg;
        NetworkManager->SendPacket(ECmdId::LIST_ROOM, Pkg);
    }
    else {
		// offline mode
    }
}

void URoomManager::RequestJoinRoom(int32 roomId)
{
    if (NetworkManager) {
        game::net::JoinRoomRequest Pkg;
        Pkg.set_room_id(roomId);
        NetworkManager->SendPacket(ECmdId::JOIN_ROOM, Pkg);
    }
}

bool URoomManager::IsMyRoom() const
{
    UPlayerInfoManager* PlayerInfoMgr = UPlayerInfoManager::Get(GetWorld());
    if (!PlayerInfoMgr) {
        return false;
    }
    return CurrentRoomData.OwnerId == PlayerInfoMgr->GetUserId();
}

void URoomManager::RequestLeaveRoom()
{
    if (NetworkManager && NetworkManager->IsConnected()) {
        game::net::Empty Pkg;
        NetworkManager->SendPacket(ECmdId::LEAVE_ROOM, Pkg);
    }
    else {
		// offline mode
		CurrentRoomData = FRoomData(); // reset room data
		OnRoomInfoReceived.Broadcast();
    }
}

void URoomManager::RequestStartGame()
{
    if (NetworkManager && NetworkManager->IsConnected()) {
        game::net::Empty Pkg;
        NetworkManager->SendPacket(ECmdId::START_GAME, Pkg);
    }
    else {
		// offline mode
        UGameInstance* GI = GetWorld()->GetGameInstance();
        if (!GI) return;

        int NumBotTeam1 = 0;
        int NumBotTeam2 = 0;

        for (int32 i = 0; i < CurrentRoomData.Players.Num(); ++i)
        {
            auto Info = CurrentRoomData.Players[i];

            if (Info.bIsBot)
            {
                if (i < 5)
                {
                    ++NumBotTeam1;
                }
                else
                {
                    ++NumBotTeam2;
                }
            }
        }

        UE_LOG(LogTemp, Warning, TEXT("Starting Game with %d bots in Team 1 and %d bots in Team 2"), NumBotTeam1, NumBotTeam2);


        FString Options(TEXT("?listen?game=/Game/Main/Core/GM_Spike.GM_Spike_C"));

        Options += FString::Printf(TEXT("?BotT1=%d"), NumBotTeam1);
        Options += FString::Printf(TEXT("?BotT2=%d"), NumBotTeam2);
        if (CurrentRoomData.Mode == EMatchMode::Spike)
        {
            const FName MapName(TEXT("/Game/Main/Maps/GhostMallMap"));
            UGameplayStatics::OpenLevel(this, MapName, true, Options);
            return;
        }
        else if (CurrentRoomData.Mode == EMatchMode::Zombie)
        {
            const FName MapName(TEXT("/Game/Main/Maps/GhostMallMap"));
            UGameplayStatics::OpenLevel(this, MapName, true, Options);
            return;
        }
        UE_LOG(LogTemp, Warning, TEXT("Start Game Clicked"));
        UGameplayStatics::OpenLevel(
            this,
            FName(TEXT("Main/Maps/L_PlayGround"))
        );
    }
}

void URoomManager::RequestChangeGameMode(EMatchMode GameMode) {
    if (NetworkManager && NetworkManager->IsConnected()) {
        game::net::ChangeGameModeRequest Pkg;
		Pkg.set_game_mode(static_cast<int32>(GameMode));
        NetworkManager->SendPacket(ECmdId::ROOM_UPDATE_GAME_MODE, Pkg);
    }
    else {
		// offline mode
		CurrentRoomData.Mode = GameMode;
		OnUpdateRoomGameMode.Broadcast();
    }
}

void URoomManager::RequestChangeHostType(bool bIsSelfHost) {
    if (NetworkManager && NetworkManager->IsConnected()) {
        game::net::ChangeHostTypeRequest Pkg;
        Pkg.set_is_self_host(bIsSelfHost);
        NetworkManager->SendPacket(ECmdId::ROOM_UPDATE_HOST_TYPE, Pkg);
    }
    else {
		// offline mode
        CurrentRoomData.bIsSelfHost = bIsSelfHost;
		OnUpdateRoomHostType.Broadcast();
    }
}

void URoomManager::RequestCreateRoom() {
    if (NetworkManager && NetworkManager->IsConnected()) {
        game::net::Empty Empty;
        NetworkManager->SendPacket(ECmdId::CREATE_ROOM, Empty);
	}
    else {
		UE_LOG(LogTemp, Warning, TEXT("URoomManager::RequestCreateRoom: NetworkManager is not connected"));
        CreateOfflineRoom();
    }
}

void URoomManager::CreateOfflineRoom()
{
    CurrentRoomData = FRoomData();
    CurrentRoomData.bIsActive = true;
    CurrentRoomData.RoomId = 0;
    CurrentRoomData.OwnerId = UPlayerInfoManager::Get(GetWorld())->GetUserId();
    CurrentRoomData.Mode = EMatchMode::Spike;
    CurrentRoomData.bIsSelfHost = true;

    CurrentRoomData.Players.Reserve(10);

    for (int i = 0; i < 10; ++i)
    {
        PlayerRoomInfo Info;
        if (i == 0) {
            Info.PlayerName = UPlayerInfoManager::Get(GetWorld())->GetPlayerName();
            Info.PlayerId = UPlayerInfoManager::Get(GetWorld())->GetUserId();
			Info.Avatar = UPlayerInfoManager::Get(GetWorld())->GetAvatar();
            Info.bIsBot = false;
        }
        else {
            Info.PlayerName = TEXT("");
            Info.PlayerId = FGameConstants::EMPTY_PLAYER_ID;
            Info.bIsBot = false;
        }
        CurrentRoomData.Players.Add(Info);
    }

    OnRoomInfoReceived.Broadcast();
}

void URoomManager::HandleGameStarted(const std::string& payload)
{
    UE_LOG(LogTemp, Warning, TEXT("URoomManager::HandleGameStarted: Game is starting..."));
    UGameInstance* GI = GetWorld()->GetGameInstance();
    if (!GI) return;

    // Turn on loading scene

    if (!CurrentRoomData.bIsSelfHost) {
        UE_LOG(LogTemp, Warning, TEXT("URoomManager::HandleGameStarted: Not self-host, skipping host startup."));
        return;
	}

    if (IsMyRoom() == false) {
        UE_LOG(LogTemp, Warning, TEXT("URoomManager::HandleGameStarted: Not my room, skipping host startup."));
        return;
	}

    UNetBoostrapSubsystem* NetSubsystem =
        GI->GetSubsystem<UNetBoostrapSubsystem>();
    if (!NetSubsystem) return;
    NetSubsystem->StartSelfHost(
        TEXT("/Game/Main/Maps/GhostMallMap"),
        FString::FromInt(CurrentRoomData.RoomId),
        CurrentRoomData.JoinKey
    );
}

void URoomManager::NotifySelfHostReady()
{
    if (NetworkManager && NetworkManager->IsConnected()) {
        game::net::Empty Pkg;
        NetworkManager->SendPacket(ECmdId::SELFHOST_READY, Pkg);
    }
}

void URoomManager::HandleSelfHostReady(const std::string& payload)
{
    UE_LOG(LogTemp, Warning, TEXT("URoomManager::HandleSelfHostReady: Join Host now."));
    if (IsMyRoom()) {
        UE_LOG(LogTemp, Warning, TEXT("URoomManager::HandleSelfHostReady: Is self-host, no need to join."));
        return;
    }

    // Turn off loading scene
    // Travel to game map as client
    UGameInstance* GI = GetWorld()->GetGameInstance();
    if (!GI) return;
    UNetBoostrapSubsystem* NetSubsystem =
        GI->GetSubsystem<UNetBoostrapSubsystem>();
    if (!NetSubsystem) return;
        
    NetSubsystem->JoinSelfHostByMatch(
        FString::FromInt(CurrentRoomData.RoomId),
        CurrentRoomData.JoinKey
	);
}