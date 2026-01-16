// Fill out your copyright notice in the Description page of Project Settings.


#include "Lobby/RoomPlayerSlotUI.h"
#include "Lobby/PlayerInfoManager.h"
#include "Lobby/RoomManager.h"

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

void URoomPlayerSlotUI::SetPlayerInfo(PlayerRoomInfo Info, int InSlotIdx)
{
	CachedPlayerInfo = Info;
	SlotIdx = InSlotIdx;
	// log UI
	UE_LOG(LogTemp, Warning, TEXT("RoomPlayerSlotUI: Setting player info: ID=%d, Name=%s"), Info.PlayerId, *Info.PlayerName);
	
	if (Info.PlayerId == FGameConstants::EMPTY_PLAYER_ID) // -1 indicates empty slot
	{
		EmptyPn->SetVisibility(ESlateVisibility::Visible);
		AvatarImg->SetVisibility(ESlateVisibility::Collapsed);
		NameLb->SetVisibility(ESlateVisibility::Collapsed);
		DeleteBtn->SetVisibility(ESlateVisibility::Collapsed);
		SwitchBtn->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		EmptyPn->SetVisibility(ESlateVisibility::Collapsed);
		AvatarImg->SetVisibility(ESlateVisibility::Visible);
		NameLb->SetVisibility(ESlateVisibility::Visible);
		NameLb->SetText(FText::FromString(Info.PlayerName));
		
		bool bIsMe = Info.PlayerId == UPlayerInfoManager::Get(GetWorld())->GetUserId();
		SwitchBtn->SetVisibility(bIsMe ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
		DeleteBtn->SetVisibility(bIsMe ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
	}
}

bool URoomPlayerSlotUI::IsEmpty() const
{
	return CachedPlayerInfo.PlayerId == FGameConstants::EMPTY_PLAYER_ID;
}

void URoomPlayerSlotUI::OnSwitchClicked()
{
	// Handle switch button click
	UE_LOG(LogTemp, Warning, TEXT("Switch button clicked for player: %s"), *CachedPlayerInfo.PlayerName);
	URoomManager* RoomMgr = GetWorld()->GetGameInstance()->GetSubsystem<URoomManager>();
	if (RoomMgr)
	{
		RoomMgr->RequestSwitchSlot(SlotIdx);
	}
}

void URoomPlayerSlotUI::OnDeleteClicked()
{
	// Handle delete button click
	UE_LOG(LogTemp, Warning, TEXT("Delete button clicked for player: %s"), *CachedPlayerInfo.PlayerName);
	URoomManager* RoomMgr = GetWorld()->GetGameInstance()->GetSubsystem<URoomManager>();
	if (RoomMgr)
	{
		RoomMgr->RequestKickPlayer(CachedPlayerInfo.PlayerId);
	}
}