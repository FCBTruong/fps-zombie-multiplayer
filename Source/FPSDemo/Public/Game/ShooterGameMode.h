// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/WeaponBase.h"
#include "GameFramework/GameMode.h"
#include "Controllers/BotAIController.h"
#include "ShooterGameMode.generated.h"

/**
 * 
 */
UCLASS()
class FPSDEMO_API AShooterGameMode : public AGameMode
{
	GENERATED_BODY()
public:
	virtual void StartPlay() override;
	virtual void NotifyPlayerKilled(class AController* Killer, class AController* Victim, class UWeaponData* DamageCauser = nullptr, bool bWasHeadShot = false);
	virtual void AssignPlayerTeam(APlayerController* NewPlayer);
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	virtual FString InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal) override;
	virtual void RestartPlayer(AController* NewPlayer) override;
	virtual void ResetPlayers();
	virtual ABotAIController* SpawnBot(FName TeamID);
	virtual bool CheckAllTeamDead(FName TeamID);
	virtual void AutoBuyForBots();
	virtual void SavePlayersGunsForNextRound();
	virtual void CleanPawnsOnMap();
protected:
    virtual void PostLogin(APlayerController* NewPlayer) override;

	FTimerHandle RoundStartTimer;
	bool bRoundInProgress = false;
	TArray<ABotAIController*> BotControllers;
};
