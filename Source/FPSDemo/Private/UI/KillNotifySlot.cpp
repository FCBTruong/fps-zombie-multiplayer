// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/KillNotifySlot.h"


void UKillNotifySlot::SetInfo(const FString& KillerName, const FString& VictimName, UTexture2D* WeaponTex, bool bIsHeadShot)
{
	if (KillerLb)
	{
		KillerLb->SetText(FText::FromString(KillerName));
	}
	if (VictimLb)
	{
		VictimLb->SetText(FText::FromString(VictimName));
	}
	if (WeaponIcon && WeaponTex)
	{
		WeaponIcon->SetBrushFromTexture(WeaponTex);
	}
	if (HeadShotIcon)
	{
		HeadShotIcon->SetVisibility(
			bIsHeadShot ? ESlateVisibility::Visible : ESlateVisibility::Collapsed
		);
	}
}