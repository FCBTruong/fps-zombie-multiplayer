// Fill out your copyright notice in the Description page of Project Settings.


#include "Lobby/RoomSlotUI.h"
#include "Lobby/RoomManager.h"

void URoomSlotUI::Init(FRoomData Data) {
	CachedRoomData = Data;
}

void URoomSlotUI::NativeConstruct()
{
	Super::NativeConstruct();

	if (JoinBtn)
	{
		JoinBtn->OnClicked.AddDynamic(this, &URoomSlotUI::OnJoinBtnClicked);
	}
}

void URoomSlotUI::OnJoinBtnClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("Join Button Clicked"));
	URoomManager::Get(GetWorld())->RequestJoinRoom(CachedRoomData.RoomId);
}
