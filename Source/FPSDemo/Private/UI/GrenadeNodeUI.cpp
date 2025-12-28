// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/GrenadeNodeUI.h"
#include "Game/ItemsManager.h"
#include "Items/ItemConfig.h"

void UGrenadeNodeUI::UpdateIcon(EItemId ItemId)
{
	CurItemId = ItemId;
	UE_LOG(LogTemp, Warning, TEXT("Updating grenade icon for ItemId: %d"), static_cast<int32>(ItemId));

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