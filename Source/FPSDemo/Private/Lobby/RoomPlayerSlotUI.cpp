// Fill out your copyright notice in the Description page of Project Settings.


#include "Lobby/RoomPlayerSlotUI.h"

void URoomPlayerSlotUI::NativeConstruct()
{
	Super::NativeConstruct();
	// Additional initialization if needed

	if (SwitchBtn)
	{
		SwitchBtn->OnClicked.AddDynamic(this, &URoomPlayerSlotUI::OnSwitchClicked);
	}
	if (DeleteBtn)
	{
		DeleteBtn->OnClicked.AddDynamic(this, &URoomPlayerSlotUI::OnDeleteClicked);
	}
}

void URoomPlayerSlotUI::SetPlayerInfo(PlayerRoomInfo Info)
{
	CachedPlayerInfo = Info;
	if (Info.PlayerId == -1) // -1 indicates empty slot
	{
		EmptyPn->SetVisibility(ESlateVisibility::Visible);
		AvatarImg->SetVisibility(ESlateVisibility::Collapsed);
		NameLb->SetVisibility(ESlateVisibility::Collapsed);
		DeleteBtn->SetVisibility(ESlateVisibility::Collapsed);
	}
	else
	{
		EmptyPn->SetVisibility(ESlateVisibility::Collapsed);
		AvatarImg->SetVisibility(ESlateVisibility::Visible);
		NameLb->SetVisibility(ESlateVisibility::Visible);
		NameLb->SetText(FText::FromString(Info.PlayerName));
		DeleteBtn->SetVisibility(ESlateVisibility::Visible);
	}
}

bool URoomPlayerSlotUI::IsEmpty() const
{
	return CachedPlayerInfo.PlayerId == -1;
}

void URoomPlayerSlotUI::OnSwitchClicked()
{
	// Handle switch button click
	UE_LOG(LogTemp, Warning, TEXT("Switch button clicked for player: %s"), *CachedPlayerInfo.PlayerName);
}

void URoomPlayerSlotUI::OnDeleteClicked()
{
	// Handle delete button click
	UE_LOG(LogTemp, Warning, TEXT("Delete button clicked for player: %s"), *CachedPlayerInfo.PlayerName);
	// temp reset player info to empty
	SetPlayerInfo(PlayerRoomInfo{ TEXT(""), -1 });

	OnDeletePlayer.Broadcast(CachedPlayerInfo.PlayerId);
}