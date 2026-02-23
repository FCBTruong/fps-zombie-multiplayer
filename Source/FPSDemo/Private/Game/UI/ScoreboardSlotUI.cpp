// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/UI/ScoreboardSlotUI.h"
#include "Game/Characters/BaseCharacter.h"
#include "Game/Framework/MyPlayerState.h"

void UScoreboardSlotUI::NativeConstruct()
{
	Super::NativeConstruct();
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(
			RefreshTimerHandle,
			this,
			&UScoreboardSlotUI::RefreshDynamicInfo,
			0.25f,
			true
		);
	}
}

void UScoreboardSlotUI::NativeDestruct()
{
	Super::NativeDestruct();
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(RefreshTimerHandle);
	}
}

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
	HighlightImg->SetVisibility(isMe ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	CachedPS = PS;

	RefreshDynamicInfo();
}

void UScoreboardSlotUI::RefreshDynamicInfo()
{
	const AMyPlayerState* PS = CachedPS.Get();
	if (!PS) return;

	// Ping
	if (PingLb)
	{
		const float PingMs = PS->GetPingInMilliseconds();
		PingLb->SetText(FText::FromString(FString::Printf(TEXT("%.0f ms"), PingMs)));
	}

	// Dead/alive opacity
	if (BackgroundImg)
	{
		bool bIsDead = false;
		if (ABaseCharacter* PSChar = Cast<ABaseCharacter>(PS->GetPawn()))
		{
			bIsDead = PSChar->IsDead();
		}

		BackgroundImg->SetOpacity(bIsDead ? 0.1f : 0.4f);
	}
}