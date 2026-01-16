// Fill out your copyright notice in the Description page of Project Settings.


#include "Lobby/RoomSlotUI.h"

void URoomSlotUI::Init(FRoomData Data) {

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
}
