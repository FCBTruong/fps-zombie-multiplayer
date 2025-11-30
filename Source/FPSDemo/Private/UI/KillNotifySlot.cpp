// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/KillNotifySlot.h"


void UKillNotifySlot::SetInfo(const FString& KillerName, const FString& VictimName, UWeaponData* WeaponConf, bool bIsHeadShot)
{
	UTexture2D* WeaponTex = nullptr;
	if (WeaponConf)
	{
		WeaponTex = WeaponConf->Icon;

		if (WeaponConf->WeaponType == EWeaponTypes::Firearm) {
			if (WeaponConf->WeaponSubType == EWeaponSubTypes::Rifle) {
				WeaponIcon->SetDesiredSizeOverride(FVector2D(120.f, 120.f));
			}
		}
	}

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