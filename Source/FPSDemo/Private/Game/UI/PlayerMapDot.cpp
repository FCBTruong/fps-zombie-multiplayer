// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/UI/PlayerMapDot.h"

void UPlayerMapDot::NativeConstruct()
{
	Super::NativeConstruct();
	SpikeIcon->SetVisibility(ESlateVisibility::Hidden);
	DeadIcon->SetVisibility(ESlateVisibility::Hidden);
}
void UPlayerMapDot::UpdateData(bool bIsMe, bool bIsDead, bool bHasSpike)
{
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
	LightScanIcon->SetVisibility(bIsMe ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}

void UPlayerMapDot::SetIsTeammateVisual(bool bIsTeammate)
{
	if (!Dot) return;

	Dot->SetColorAndOpacity(bIsTeammate
		? FLinearColor(0.028f, 0.484f, 0.073f, 1.f)
		: FLinearColor::Red);

	if (!bIsTeammate) {
		LightScanIcon->SetVisibility(ESlateVisibility::Hidden);
	}
}