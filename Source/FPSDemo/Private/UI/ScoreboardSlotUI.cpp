// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ScoreboardSlotUI.h"
#include "Characters/BaseCharacter.h"

void UScoreboardSlotUI::Setup(const AMyPlayerState* PS, bool isMe)
{
	if (!PS) return;
	if (PlayerNameLb)
	{
		PlayerNameLb->SetText(FText::FromString(PS->GetPlayerName()));
	}
	if (KillLb)
	{
		KillLb->SetText(FText::AsNumber(PS->GetKills()));
	}
	if (DeathLb)
	{
		DeathLb->SetText(FText::AsNumber(PS->GetDeaths()));
	}
	if (AssistLb)
	{
		AssistLb->SetText(FText::AsNumber(PS->GetAssists()));
	}

	if (PS->GetTeamId() == ETeamId::Defender) {
		BackgroundImg->SetColorAndOpacity(FLinearColor(0.3f, 0.65f, 1.0f, 0.6f)); // Blueish
	}
	else if (PS->GetTeamId() == ETeamId::Attacker) {
		BackgroundImg->SetColorAndOpacity(FLinearColor(0.617f, 0.184f, 0.036f, 0.6f)); // Reddish
	}

	bool bIsDead = false;
	ABaseCharacter* PSChar = Cast<ABaseCharacter>(PS->GetPawn());
	if (PSChar)
	{
		bIsDead = PSChar->IsDead();
	}
	if (bIsDead) {
		BackgroundImg->SetOpacity(0.1f); // faded
	}
	else {
		BackgroundImg->SetOpacity(0.4f); // faded
	}

	
	HighlightImg->SetVisibility(isMe ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}