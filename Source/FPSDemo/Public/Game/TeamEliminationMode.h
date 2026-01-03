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
	void EndGame(FName WinningTeam);
	void CheckRoundEnd();
	void EndRound(FName WinningTeam);
	
	static constexpr int MaxRoundsToWin = 5;
	virtual void NotifyPlayerKilled(class AController* Killer, ABaseCharacter* Victim, const UItemConfig* DamageCauser, bool bWasHeadShot) override;
};
