// Fill out your copyright notice in the Description page of Project Settings.


#include "Modules/Lobby/RoomPlayerSlotUI.h"
#include "Shared/System/PlayerInfoManager.h"
#include "Modules/Lobby/RoomManager.h"
#include "Shared/Utils/GameUtils.h"
#include "Shared/System/ContentRegistrySubsystem.h"

void URoomPlayerSlotUI::NativeConstruct()
{
	Super::NativeConstruct();

	SwitchBtn->OnClicked.AddDynamic(this, &URoomPlayerSlotUI::OnSwitchClicked);
	DeleteBtn->OnClicked.AddDynamic(this, &URoomPlayerSlotUI::OnDeleteClicked);
}

void URoomPlayerSlotUI::SetPlayerInfo(FPlayerRoomInfo Info, int InSlotIdx, int OwnerId, bool IsGuestMode)
{
	CachedPlayerInfo = Info;
	SlotIdx = InSlotIdx;
	// log UI
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
	URoomManager* RoomMgr = GetWorld()->GetGameInstance()->GetSubsystem<URoomManager>();
	if (RoomMgr)
	{
		RoomMgr->RequestSwitchSlot(SlotIdx);
	}
}

void URoomPlayerSlotUI::OnDeleteClicked()
{
	// Handle delete button click
	URoomManager* RoomMgr = GetWorld()->GetGameInstance()->GetSubsystem<URoomManager>();
	if (RoomMgr)
	{
		RoomMgr->RequestKickPlayer(CachedPlayerInfo.PlayerId);
	}
}
