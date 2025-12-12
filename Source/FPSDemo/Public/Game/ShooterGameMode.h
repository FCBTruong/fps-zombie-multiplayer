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
	virtual void AddPlayer(APlayerController* NewPlayer);
	virtual FString InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal) override;
	void RestartPlayer(AController* NewPlayer) override;
	void ResetPlayers();
	ABotAIController* SpawnBot(FName TeamID);
	bool CheckAllTeamDead(FName TeamID);
protected:
    virtual void PostLogin(APlayerController* NewPlayer) override;

	FTimerHandle RoundStartTimer;
	bool bRoundInProgress = false;
};
