// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Network/PacketListener.h"
#include "PlayerInfoManager.generated.h"

namespace PlayerLocalInfoConst
{
	static constexpr const TCHAR* SLOT = TEXT("PlayerLocalInfo");
	static constexpr int32 USER_INDEX = 0;
}
/**
 * 
 */
UCLASS()
class FPSDEMO_API UPlayerInfoManager : public UGameInstanceSubsystem, public IPacketListener
{
	GENERATED_BODY()

private:
	FString GuestId;
	FString PlayerName;
	int UserId;
	FString Avatar;

	void LoadOrCreateLocalInfo();

public:
	UPlayerInfoManager();
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	void SetGuestId(const FString& InGuestId);
	FString GetGuestId() const;
	void SetPlayerName(const FString& InPlayerName);
	FString GetPlayerName() const;
	void SetUserId(int InUserId);
	int GetUserId() const;
	void SetAvatar(const FString& InAvatar);
	FString GetAvatar() const;
	virtual void OnPacketReceived(
		ECmdId CmdId,
		const std::string& Payload
	) override;

	static UPlayerInfoManager* Get(UWorld* World);
};
