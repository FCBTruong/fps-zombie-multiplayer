// Fill out your copyright notice in the Description page of Project Settings.


#include "Modules/Lobby/RoomSlotUI.h"
#include "Modules/Lobby/RoomManager.h"

void URoomSlotUI::Init(FRoomData Data) {
	CachedRoomData = Data;

	FString ModeString;
	switch (Data.Mode)
	{
	case EMatchMode::DeathMatch:
		ModeString = TEXT("Deathmatch");
		break;
	case EMatchMode::Spike:
		ModeString = TEXT("Spike");
		break;
	case EMatchMode::Zombie:
		ModeString = TEXT("Zombie");
		break;
	default:
		ModeString = TEXT("Unknown");
		break;
	}
	int PlayerCount = 0;
	for (const auto& PlayerInfo : Data.Players)
	{
		if (PlayerInfo.PlayerId != FGameConstants::EMPTY_PLAYER_ID)
		{
			PlayerCount++;
		}
	}
	FString PlayerCountString = FString::Printf(TEXT("%d/10"), PlayerCount);

	const FString HostIndicator = CachedRoomData.bIsSelfHost ? TEXT("SelfHost") : TEXT("DedicatedServer");

	const FString RoomName = FString::Printf(
		TEXT("Room %d|%s| %s | Players %s"),
		Data.RoomId,
		*ModeString,
		*HostIndicator,
		*PlayerCountString
	);

	RoomLb->SetText(FText::FromString(RoomName));
}

void URoomSlotUI::NativeConstruct()
{
	Super::NativeConstruct();

	JoinBtn->OnClicked.AddDynamic(this, &URoomSlotUI::OnJoinBtnClicked);
}

void URoomSlotUI::OnJoinBtnClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("Join Button Clicked"));
	URoomManager::Get(GetWorld())->RequestJoinRoom(CachedRoomData.RoomId);
}
