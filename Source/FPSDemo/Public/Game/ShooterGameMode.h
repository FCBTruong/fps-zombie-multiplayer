// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "Controllers/BotAIController.h"
#include "Bot/BotStateManager.h"
#include "ShooterGameMode.generated.h"

class UItemConfig;
class AShooterGameState;
/**
 * 
 */
UCLASS()
class FPSDEMO_API AShooterGameMode : public AGameMode
{
	GENERATED_BODY()
public:
	AShooterGameMode();

	virtual void StartPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void NotifyPlayerKilled(class AController* Killer, class AController* Victim, const UItemConfig* DamageCauser = nullptr, bool bWasHeadShot = false);
	virtual void AssignPlayerTeam(APlayerController* NewPlayer);
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	virtual FString InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal) override;
	virtual void RestartPlayer(AController* NewPlayer) override;
	virtual void ResetPlayers();
	virtual ABotAIController* SpawnBot(FName TeamID);
	virtual bool CheckAllTeamDead(FName TeamID);
	virtual void AutoBuyForBots();
	virtual void SavePlayersGunsForNextRound();
	AShooterGameState* GetShooterGS() const;
	virtual void RegisterCorpse(AActor* Corpse);
	virtual void CleanupCorpses();
protected:
    virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual bool ReadyToStartMatch_Implementation() override;
	virtual void HandleMatchHasStarted() override;
	virtual void StartRound();

	FTimerHandle RoundStartTimer;
	bool bRoundInProgress = false;
	TUniquePtr<BotStateManager> BotManager;

	UPROPERTY()
	TArray<TWeakObjectPtr<AActor>> Corpses;

	bool bRoundStarted = false;

	FTimerHandle TryStartMatchHandle;
};
