// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/UI/ScoreboardUI.h"
#include "Game/Framework/MyPlayerState.h"
#include "Game/Framework/ShooterGameState.h"


void UScoreboardUI::NativeConstruct()
{
	Super::NativeConstruct();
}

void UScoreboardUI::UpdateScoreboard(class AShooterGameState* GameState)
{
	if (!ScoreboardSlotClass) return;
	ScoreboardBox->ClearChildren();
	ScoreboardBoxA->ClearChildren();
	ScoreboardBoxB->ClearChildren();

	if (!GameState) return;

	const APlayerController* PC = GetOwningPlayer();
	const AMyPlayerState* LocalPS =
		PC ? Cast<AMyPlayerState>(PC->PlayerState) : nullptr;

	TArray<const AMyPlayerState*> SortedPlayers;

	auto MatchMode = GameState->GetMatchMode();
	for (APlayerState* PS : GameState->PlayerArray)
	{
		if (const AMyPlayerState* MyPS = Cast<AMyPlayerState>(PS))
		{
			SortedPlayers.Add(MyPS);
		}
	}

	if (MatchMode == EMatchMode::Spike) {
		Divide->SetVisibility(ESlateVisibility::Visible);
	}
	else {
		Divide->SetVisibility(ESlateVisibility::Hidden);
	}

	// 2. Sort by kills (DESC)
	SortedPlayers.Sort([](const AMyPlayerState& A, const AMyPlayerState& B)
		{
			return A.GetKills() > B.GetKills();
		});

	for (const AMyPlayerState* MyPS : SortedPlayers){
		UScoreboardSlotUI* CvSlot = CreateWidget<UScoreboardSlotUI>(GetWorld(), ScoreboardSlotClass);
		if (CvSlot)
		{
			bool IsMe = (MyPS == LocalPS);
			CvSlot->Setup(MyPS, IsMe);

			if (MatchMode == EMatchMode::DeathMatch
				|| MatchMode == EMatchMode::Zombie) {
				ScoreboardBox->AddChild(CvSlot);
				continue;
			}
			if (MyPS->GetTeamId() == ETeamId::Attacker) {
				ScoreboardBoxA->AddChild(CvSlot);
			}
			else if (MyPS->GetTeamId() == ETeamId::Defender) {
				ScoreboardBoxB->AddChild(CvSlot);
			}
		}
	}
}