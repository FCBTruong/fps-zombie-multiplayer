// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ScoreboardUI.h"
#include "Controllers/MyPlayerState.h"
#include "Game/ShooterGameState.h"


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

	const APlayerController* PC = GetOwningPlayer();
	const AMyPlayerState* LocalPS =
		PC ? Cast<AMyPlayerState>(PC->PlayerState) : nullptr;

	TArray<const AMyPlayerState*> SortedPlayers;

	for (APlayerState* PS : GameState->PlayerArray)
	{
		if (const AMyPlayerState* MyPS = Cast<AMyPlayerState>(PS))
		{
			SortedPlayers.Add(MyPS);
		}
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

			if (MyPS->GetTeamId() == ETeamId::Attacker) {
				ScoreboardBoxA->AddChild(CvSlot);
			}
			else if (MyPS->GetTeamId() == ETeamId::Defender) {
				ScoreboardBoxB->AddChild(CvSlot);
			}
		}
	}
}