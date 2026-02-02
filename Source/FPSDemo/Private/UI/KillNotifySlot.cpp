// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/KillNotifySlot.h"
#include "Items/ItemConfig.h"
#include "Items/FirearmConfig.h"

void UKillNotifySlot::SetInfo(const AMyPlayerState* Killer, const AMyPlayerState* Victim, const UItemConfig* WeaponConf, bool bIsHeadShot)
{
	const FString KillerName = Killer ? Killer->GetPlayerName() : TEXT("");
	const FString VictimName = Victim ? Victim->GetPlayerName() : TEXT("Unknown");

	UTexture2D* WeaponTex = nullptr;
	if (WeaponConf)
	{
		WeaponTex = WeaponConf->ItemIcon;

		if (WeaponConf->GetItemType() == EItemType::Firearm) {
			const UFirearmConfig* FirearmConf = Cast<UFirearmConfig>(WeaponConf);
			if (FirearmConf->FirearmType == EFirearmType::Rifle) {
				WeaponIcon->SetDesiredSizeOverride(FVector2D(120.f, 120.f));
			}
		}
	}

    // Define colors
    const FLinearColor AttackerColor = FLinearColor(0.617f, 0.184f, 0.036, 1.0f); // Red
    const FLinearColor DefenderColor = FLinearColor(0.3f, 0.65f, 1.0f, 1.0f); // Blue

    if (KillerLb)
    {
        KillerLb->SetText(FText::FromString(KillerName));

        if (Killer)
        {
            if (Killer->GetTeamId() == ETeamId::Attacker)
            {
                KillerLb->SetColorAndOpacity(FSlateColor(AttackerColor));
            }
            else if (Killer->GetTeamId() == ETeamId::Defender)
            {
                KillerLb->SetColorAndOpacity(FSlateColor(DefenderColor));
            }
        }
    }

    if (VictimLb)
    {
        VictimLb->SetText(FText::FromString(VictimName));

        if (Victim)
        {
            if (Victim->GetTeamId() == ETeamId::Defender)
            {
                VictimLb->SetColorAndOpacity(FSlateColor(DefenderColor));
            }
            else if (Victim->GetTeamId() == ETeamId::Attacker)
            {
                VictimLb->SetColorAndOpacity(FSlateColor(AttackerColor));
            }
        }
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