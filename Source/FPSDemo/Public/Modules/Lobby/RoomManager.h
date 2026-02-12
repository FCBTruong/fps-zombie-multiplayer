// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Network/PacketListener.h"
#include "Modules/Lobby/RoomData.h"
#include "RoomManager.generated.h"

class UNetworkManager;

DECLARE_MULTICAST_DELEGATE(FOnRoomInfoReceived);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnUpdateRoomSlot, int /*SlotIdx*/);
DECLARE_MULTICAST_DELEGATE(FOnUpdateRoomList);
DECLARE_MULTICAST_DELEGATE(FOnUpdateRoomOwner);
DECLARE_MULTICAST_DELEGATE(FOnUpdateRoomGameMode);
DECLARE_MULTICAST_DELEGATE(FOnUpdateRoomHostType);
/**
 * 
 */
UCLASS()
class FPSDEMO_API URoomManager : public UGameInstanceSubsystem, public IPacketListener
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void OnPacketReceived(
		ECmdId CmdId,
		const std::string& Payload
	) override;

	void RequestKickPlayer(int PlayerId);
	void RequestSwitchSlot(int SlotIdx);
	void RequestAddBot(int team);
	void RequestRoomList();
	void RequestLeaveRoom();
	void RequestStartGame();
	void RequestChangeGameMode(EMatchMode GameMode);
	void RequestChangeHostType(bool bIsSelfHost);
	void RequestCreateRoom();

	const FRoomData& GetCurrentRoomData() {
		return CurrentRoomData;
	}
	FRoomData& GetCurrentRoomDataMutable() {
		return CurrentRoomData;
	}
	void SetCurrentRoomData(const FRoomData& NewData) {
		CurrentRoomData = NewData;
	}
	const TArray<FRoomData>& GetAvailableRooms() const
	{
		return AvailableRooms;
	}

	bool HasCurrentRoom() const {
		return CurrentRoomData.bIsActive;
	}

	void RequestJoinRoom(int32 roomId);
	bool IsMyRoom() const;

	static URoomManager* Get(UWorld* World);

	static constexpr int TEAM_1 = 0;
	static constexpr int TEAM_2 = 1;

	FOnRoomInfoReceived OnRoomInfoReceived;
	FOnUpdateRoomSlot OnUpdateRoomSlot;
	FOnUpdateRoomList OnUpdateRoomList;
	FOnUpdateRoomOwner OnUpdateRoomOwner;
	FOnUpdateRoomGameMode OnUpdateRoomGameMode;
	FOnUpdateRoomHostType OnUpdateRoomHostType;
private:
	UNetworkManager* NetworkManager = nullptr;
	FRoomData CurrentRoomData;
	TArray<FRoomData> AvailableRooms;

	void CreateOfflineRoom();
	void HandleGameStarted(const std::string& payload);
	void HandleSelfHostReady(const std::string& payload);
	void HandlePlayerSession(const std::string& payload);
	void CreateSelfHostSession(FString RoomId, FString Token);

	FTimerHandle JoinRetryTimer;
};
