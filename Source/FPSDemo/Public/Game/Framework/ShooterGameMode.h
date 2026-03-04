// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Game/AI/BotStateManager.h"
#include "ShooterGameMode.generated.h"

class UItemConfig;
class AShooterGameState;
class ABaseCharacter;
class APlayerSlot;
class ABotAIController;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnCharacterDead, ABaseCharacter*);
/**
 * 
 */
UCLASS()
class FPSDEMO_API AShooterGameMode : public AGameModeBase
{
	GENERATED_BODY()
public:
	AShooterGameMode();
	virtual bool CheckAllTeamDead(ETeamId TeamId) const;
	virtual void RegisterCorpse(AActor* Corpse);
	virtual void CleanupCorpses();
	virtual EMatchMode GetMatchMode() const {
		return EMatchMode::None;
	}
	virtual bool IsDamageAllowed(AController* Killer, AController* Victim) const;
	virtual void HandleCharacterKilled(class AController* Killer, const TArray<TWeakObjectPtr<AController>>& Assists, ABaseCharacter* Victim, const UItemConfig* DamageCauser = nullptr, bool bWasHeadShot = false);

	// Delegates
	FOnCharacterDead OnCharacterDead;
protected:
	virtual void InitGame(
		const FString& MapName,
		const FString& Options,
		FString& ErrorMessage) override;
	virtual void InitGameState() override;
	virtual void StartPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual FString InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal) override;
	virtual void PreLogin(
		const FString& Options,
		const FString& Address,
		const FUniqueNetIdRepl& UniqueId,
		FString& ErrorMessage) override;
	virtual void Logout(AController* Exiting) override;
    virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual APawn* SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform) override;

	virtual ABotAIController* SpawnBot(APlayerSlot* Slot);
	virtual void StartMatch();
	virtual void ScheduleMatchStart(int DelaySeconds);
	virtual void StartMatchFromCountdown();
	virtual void RestartAllPlayers();
	virtual void StartRound();
	virtual void EndRound(ETeamId WinningTeam);
	virtual void EndGame(ETeamId WinningTeam);
	UFUNCTION()
	void StartRoundDelayed();
	virtual FTransform GetSpawnTransformForSlot(const APlayerSlot& Slot);
	bool AreAllPlayersConnected() const;

	// Properties
	bool bRoundInProgress = false;
	bool bRoundStarted = false;
	bool bMatchStarted = false;
	int MatchStartDelayDefault = 15;
	int MatchStartDelayWhenAllJoined = 3;
	TUniquePtr<BotStateManager> BotManager;
	UPROPERTY()
	TArray<TWeakObjectPtr<AActor>> Corpses;

	// Timers
	FTimerHandle StartRoundTimerHandle;
	FTimerHandle RoundStartTimer;
	FTimerHandle MatchStartCountdownHandle;
	bool bAllowFriendlyFire = true;

	AShooterGameState* CachedGS;
};
