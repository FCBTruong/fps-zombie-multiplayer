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


private:
	UPROPERTY()
	TArray<APlayerController*> TeamA;

	UPROPERTY()
	TArray<APlayerController*> TeamB;

	FTimerHandle RoundStartTimer;
public:
	ATeamEliminationMode();

	void AddPlayer(APlayerController* NewPlayer);
	void PostLogin(APlayerController* NewPlayer) override;

    AActor* ChoosePlayerStart_Implementation(AController* Player) override;
	void StartRound();
	void ResetPlayers();
	void StartNextRound();
	void EndGame(FName WinningTeam);
	void NotifyPlayerKilled(class AController* Killer, class AController* Victim, class AActor* DamageCauser) override;
	void CheckRoundEnd();
	void EndRound(FName WinningTeam);
	void RestartPlayer(AController* NewPlayer) override;

	static constexpr int MaxRoundsToWin = 5;
};
