// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Game/ShooterGameMode.h"
#include "Controllers/MyPlayerState.h"
#include "TeamEliminationMode.generated.h"


/**
 * 
 */
UCLASS()
class FPSDEMO_API ATeamEliminationMode : public AShooterGameMode
{
	GENERATED_BODY()


protected:
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
public:
	ATeamEliminationMode();
	virtual void BeginPlay() override;
	void PostLogin(APlayerController* NewPlayer) override;
	virtual void StartRound() override;
	
	void StartNextRound();
	virtual void EndGame(ETeamId WinningTeam) override;
	void CheckRoundEnd();
	void EndRound(ETeamId WinningTeam);
	
	static constexpr int MaxRoundsToWin = 5;
	virtual void OnCharacterKilled(class AController* Killer, ABaseCharacter* Victim, const UItemConfig* DamageCauser, bool bWasHeadShot) override;

	virtual EMatchMode GetMatchMode() const {
		return EMatchMode::TeamElimination;
	}
};
