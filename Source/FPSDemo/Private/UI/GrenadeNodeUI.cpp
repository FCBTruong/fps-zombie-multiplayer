// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/GrenadeNodeUI.h"
#include "Weapons/WeaponData.h"
#include "Game/GameManager.h"

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

		UGameManager* GMR = GetWorld()->GetGameInstance()->GetSubsystem<UGameManager>();
		UWeaponData* WeaponConf = GMR->GetWeaponDataById(ItemId); 
		if (WeaponConf)
		{

			Icon->SetBrushFromTexture(WeaponConf->Icon);
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