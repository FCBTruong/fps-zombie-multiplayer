// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Network/PacketListener.h"
#include "Lobby/RoomData.h"
#include "RoomManager.generated.h"

class UNetworkManager;

DECLARE_MULTICAST_DELEGATE(FOnRoomInfoReceived);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnUpdateRoomSlot, int /*SlotIdx*/);
DECLARE_MULTICAST_DELEGATE(FOnUpdateRoomList);
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
	FRoomData GetCurrentRoomData() {
		return CurrentRoomData;
	}
	TArray<FRoomData> GetAvailableRooms() {
		return AvailableRooms;
	}

	static URoomManager* Get(UWorld* World);

	static constexpr int TEAM_1 = 0;
	static constexpr int TEAM_2 = 1;

	FOnRoomInfoReceived OnRoomInfoReceived;
	FOnUpdateRoomSlot OnUpdateRoomSlot;
	FOnUpdateRoomList OnUpdateRoomList;
private:
	UNetworkManager* NetworkManager = nullptr;
	FRoomData CurrentRoomData;
	TArray<FRoomData> AvailableRooms;
};
