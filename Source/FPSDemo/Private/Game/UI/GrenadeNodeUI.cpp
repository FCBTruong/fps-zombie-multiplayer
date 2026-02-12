// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/UI/GrenadeNodeUI.h"
#include "Game/Subsystems/ItemsManager.h"
#include "Shared/Data/Items/ItemConfig.h"

void UGrenadeNodeUI::UpdateIcon(EItemId ItemId)
{
	CurItemId = ItemId;

	if (ItemId == EItemId::NONE) {
		Dot->SetVisibility(ESlateVisibility::Visible);
		Icon->SetVisibility(ESlateVisibility::Hidden);
	}
	else {
		Dot->SetVisibility(ESlateVisibility::Hidden);
		Icon->SetVisibility(ESlateVisibility::Visible);

		const UItemConfig* ItemConf = UItemsManager::Get(GetWorld())->GetItemById(ItemId);
		if (ItemConf)
		{
			if (!ItemConf->ItemIcon)
			{
				return;
			}
			Icon->SetBrushFromTexture(ItemConf->ItemIcon.Get());
		}
	}
}

void UGrenadeNodeUI::SetSelected(bool bIsSelected)
{
	if (bIsSelected) {
		Icon->SetRenderScale(FVector2D(1.2f, 1.2f));
	}
	else {
		Icon->SetRenderScale(FVector2D(1.0f, 1.0f));
	}
}