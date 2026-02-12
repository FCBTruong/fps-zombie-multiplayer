// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Network/CmdId.h"
#include "Network/PacketListener.h"
#include "ChatSubsystem.generated.h"

class UNetworkManager;
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnNewChatMessage, const FString&, const FString&);
/**
 * 
 */
UCLASS()
class FPSDEMO_API UChatSubsystem : public UGameInstanceSubsystem, public IPacketListener
{
	GENERATED_BODY()

public:
	FOnNewChatMessage OnNewChatMessage;

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void OnPacketReceived(
		ECmdId CmdId,
		const std::string& Payload
	) override;

	void SendChatMessageInRoom(const FString& Message);

private:
	void HandleChatMessage(const std::string& payload);
	UNetworkManager* NetworkManager = nullptr;
};
