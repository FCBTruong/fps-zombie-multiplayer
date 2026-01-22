// Fill out your copyright notice in the Description page of Project Settings.


#include "Lobby/RoomPlayerSlotUI.h"
#include "Lobby/PlayerInfoManager.h"
#include "Lobby/RoomManager.h"
#include "Utils/GameUtils.h"
#include "ContentRegistrySubsystem.h"

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

void URoomPlayerSlotUI::SetPlayerInfo(PlayerRoomInfo Info, int InSlotIdx, int OwnerId, bool IsGuestMode)
{
	CachedPlayerInfo = Info;
	SlotIdx = InSlotIdx;
	// log UI
	UE_LOG(LogTemp, Warning, TEXT("RoomPlayerSlotUI: Setting player info: ID=%d, Name=%s"), Info.PlayerId, *Info.PlayerName);
	
	OwnerIcon->SetVisibility(ESlateVisibility::Collapsed);
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
		SwitchBtn->SetVisibility(ESlateVisibility::Collapsed);

		bool bIsRoomOwner = Info.PlayerId == OwnerId;
		if (bIsRoomOwner) {
			OwnerIcon->SetVisibility(ESlateVisibility::Visible);
		}

		if (bIsMe)
		{
			FSlateBrush Brush = AvatarImg->GetBrush();
			Brush.OutlineSettings.Color = FLinearColor(
				0x63 / 255.0f,
				0x32 / 255.0f,
				0x1A / 255.0f,
				1.0f
			);
			AvatarImg->SetBrush(Brush);
		}
		else
		{
			FSlateBrush Brush = AvatarImg->GetBrush();
			Brush.OutlineSettings.Color = FLinearColor::Black;
			AvatarImg->SetBrush(Brush);
		}

		if (IsGuestMode) {
			DeleteBtn->SetVisibility(ESlateVisibility::Collapsed);
		}
		else {
			DeleteBtn->SetVisibility(bIsMe ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);
		}
		
		UContentRegistrySubsystem* Registry =
			GetGameInstance()->GetSubsystem<UContentRegistrySubsystem>();
		AvatarImg->SetBrushFromTexture(
			Registry->GetAvatarTextureById(Info.Avatar));
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
