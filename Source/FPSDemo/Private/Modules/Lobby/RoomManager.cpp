// Fill out your copyright notice in the Description page of Project Settings.


#include "Modules/Lobby/RoomManager.h"
#include "Network/NetworkManager.h"
#include "Shared/System/PlayerInfoManager.h"
#include "Kismet/GameplayStatics.h"
#include "Game/GameManager.h"
#include "Game/Data/MatchInfo.h"

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
		CurrentRoomData.bEnableDedicatedServer = RoomInfoPkg.enable_dedicated_server();
		CurrentRoomData.bEnableSelfHost = RoomInfoPkg.enable_selfhost();

        CurrentRoomData.Players.Empty();    
        for (int i = 0; i < RoomInfoPkg.players_size(); ++i) {
            const auto& ProtoPlayerInfo = RoomInfoPkg.players(i);
            FPlayerRoomInfo Info;
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
                FPlayerRoomInfo Info;
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
            FPlayerRoomInfo Info;

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
            FPlayerRoomInfo Info;
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
    case ECmdId::PLAYER_SESSION:
    {
        HandlePlayerSession(Payload);
        break;
    }
    case ECmdId::SELFHOST_START_SESSION: {
		game::net::SelfHostStartSessionReply Msg;
        if (!Msg.ParseFromString(Payload))
			return;
        const FString RoomId = UTF8_TO_TCHAR(Msg.room_id().c_str());
        const FString Token = UTF8_TO_TCHAR(Msg.token().c_str());
        CreateSelfHostSession(RoomId, Token);
        break;
    }
    case ECmdId::GAME_START_FAILED: {
		// handle game start failed, back too lobby scene
        UGameplayStatics::OpenLevel(this, FGameConstants::LEVEL_LOBBY);
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
                FPlayerRoomInfo Info;
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
                FPlayerRoomInfo Info;
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
		UGameManager* GameMgr = UGameManager::Get(GetWorld());

		// build match info
		FMatchInfo MatchInfo;
		MatchInfo.Mode = CurrentRoomData.Mode;
		MatchInfo.Players.Empty();
        for (const FPlayerRoomInfo& PlayerRoomInfo : CurrentRoomData.Players) { 
            FPlayerMatchInfo PlayerMatchInfo;
            PlayerMatchInfo.PlayerId = PlayerRoomInfo.PlayerId;
            PlayerMatchInfo.PlayerName = PlayerRoomInfo.PlayerName;
			PlayerMatchInfo.CrosshairCode = PlayerRoomInfo.CrosshairCode;
			PlayerMatchInfo.bIsBot = PlayerRoomInfo.bIsBot;
            PlayerMatchInfo.Avatar = PlayerRoomInfo.Avatar;
			MatchInfo.Players.Add(PlayerMatchInfo);
        }
        if (GameMgr) {
            GameMgr->StartMatch(MatchInfo);
        }
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
        game::net::CreateRoomRequest CreateRoomPkg;
        NetworkManager->SendPacket(ECmdId::CREATE_ROOM, CreateRoomPkg);
	}
    else {
		UE_LOG(LogTemp, Warning, TEXT("URoomManager::RequestCreateRoom: NetworkManager is not connected"));
        CreateOfflineRoom();
    }
}

void URoomManager::CreateOfflineRoom()
{
	UPlayerInfoManager* PlayerInfoMgr = UPlayerInfoManager::Get(GetWorld());
    CurrentRoomData = FRoomData();
    CurrentRoomData.bIsActive = true;
    CurrentRoomData.RoomId = 0;
    CurrentRoomData.OwnerId = PlayerInfoMgr->GetUserId();
    CurrentRoomData.Mode = EMatchMode::Spike;
    CurrentRoomData.bIsSelfHost = true;

    CurrentRoomData.Players.Reserve(10);

    for (int i = 0; i < 10; ++i)
    {
        FPlayerRoomInfo Info;
        if (i == 0) {
            Info.PlayerName = PlayerInfoMgr->GetPlayerName();
            Info.PlayerId = PlayerInfoMgr->GetUserId();
			Info.Avatar = PlayerInfoMgr->GetAvatar();
			Info.CrosshairCode = PlayerInfoMgr->GetCrosshairCode();
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

    UGameplayStatics::OpenLevel(this, FGameConstants::LEVEL_LOADING);
}

void URoomManager::HandleSelfHostReady(const std::string& payload)
{
    UE_LOG(LogTemp, Warning, TEXT("URoomManager::HandleSelfHostReady: Join Host now."));
    if (IsMyRoom()) {
        UE_LOG(LogTemp, Warning, TEXT("URoomManager::HandleSelfHostReady: Is self-host, no need to join."));
        return;
    }
}

void URoomManager::HandlePlayerSession(const std::string& payload)
{
    game::net::PlayerSessionReply Msg;
    if (!Msg.ParseFromString(payload))
        return;

	auto IpAddress = FString(Msg.ip().c_str());
	auto Port = Msg.port();
	UE_LOG(LogTemp, Warning, TEXT("URoomManager::HandlePlayerSession: Received session info. IP: %s, Port: %d"), *IpAddress, Port);
	auto SessionId = Msg.playersessionid().c_str();
	UE_LOG(LogTemp, Warning, TEXT("URoomManager::HandlePlayerSession: Player Session ID: %s"), *FString(SessionId));

    UGameInstance* GI = GetWorld()->GetGameInstance();
    if (!GI) return;
 
    FString SessionIdStr = UTF8_TO_TCHAR(SessionId);
    FString URL = FString::Printf(TEXT("%s:%d"), *IpAddress, Port);

    if (!SessionIdStr.IsEmpty())
    {
        URL += (URL.Contains(TEXT("?")) ? TEXT("&") : TEXT("?"));
        URL += FString::Printf(TEXT("PlayerSessionId=%s"), *SessionIdStr);
    }
	GI->GetFirstLocalPlayerController()->ClientTravel(URL, TRAVEL_Absolute);
}

void URoomManager::CreateSelfHostSession(FString RoomId, FString Token)
{
	UGameManager* GameMgr = UGameManager::Get(GetWorld());
    GameMgr->InitServerConfig(
        RoomId,
        Token
	);
	GameMgr->RequestMatchDataAndStart();
}