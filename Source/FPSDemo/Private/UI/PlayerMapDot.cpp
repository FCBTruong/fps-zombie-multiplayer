// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PlayerMapDot.h"


void UPlayerMapDot::UpdateData(bool bIsMe, bool bIsDead, bool bHasSpike)
{
	if (bIsMe)
	{
		Dot->SetColorAndOpacity(FLinearColor::Green);
		LightScanIcon->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		Dot->SetColorAndOpacity(FLinearColor(0.028f, 0.484f, 0.073f, 1.f));
		LightScanIcon->SetVisibility(ESlateVisibility::Hidden);
	}

	if (bIsDead)
	{
		DeadIcon->SetVisibility(ESlateVisibility::Visible);
		Arrow->SetVisibility(ESlateVisibility::Hidden);
		Dot->SetVisibility(ESlateVisibility::Hidden);
	}
	else
	{
		DeadIcon->SetVisibility(ESlateVisibility::Hidden);
		Arrow->SetVisibility(ESlateVisibility::Visible);
		Dot->SetVisibility(ESlateVisibility::Visible);
	}
	if (bHasSpike)
	{
		SpikeIcon->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		SpikeIcon->SetVisibility(ESlateVisibility::Hidden);
	}
}