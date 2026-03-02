// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/UI/ScoreboardSlotUI.h"
#include "Game/Characters/BaseCharacter.h"
#include "Game/Framework/MyPlayerState.h"

void UScoreboardSlotUI::NativeConstruct()
{
	Super::NativeConstruct();

	GetWorld()->GetTimerManager().SetTimer(
		RefreshTimerHandle,
		this,
		&UScoreboardSlotUI::RefreshDynamicInfo,
		0.25f,
		true
	);
}

void UScoreboardSlotUI::NativeDestruct()
{
	Super::NativeDestruct();

	GetWorld()->GetTimerManager().ClearTimer(RefreshTimerHandle);
}

void UScoreboardSlotUI::Setup(const AMyPlayerState* PS, bool isMe)
{
	if (!PS) return;

	PlayerNameLb->SetText(FText::FromString(PS->GetPlayerName()));
	KillLb->SetText(FText::AsNumber(PS->GetKills()));
	DeathLb->SetText(FText::AsNumber(PS->GetDeaths()));
	AssistLb->SetText(FText::AsNumber(PS->GetAssists()));

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
	const float PingMs = PS->GetPingInMilliseconds();
	PingLb->SetText(FText::FromString(FString::Printf(TEXT("%.0f ms"), PingMs)));

	// Dead/alive opacity
	bool bIsDead = false;
	if (ABaseCharacter* PSChar = Cast<ABaseCharacter>(PS->GetPawn()))
	{
		bIsDead = PSChar->IsDead();
	}

	BackgroundImg->SetOpacity(bIsDead ? 0.1f : 0.4f);
}