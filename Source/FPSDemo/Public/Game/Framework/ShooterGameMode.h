// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "Game/AI/BotAIController.h"
#include "Game/AI/BotStateManager.h"
#include "Modules/Lobby/RoomData.h"
#include "ShooterGameMode.generated.h"

class UItemConfig;
class AShooterGameState;
class ABaseCharacter;
class APlayerSlot;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnCharacterDead, ABaseCharacter*);
/**
 * 
 */
UCLASS()
class FPSDEMO_API AShooterGameMode : public AGameMode
{
	GENERATED_BODY()
public:
	AShooterGameMode();
	virtual void InitGame(
		const FString& MapName,
		const FString& Options,
		FString& ErrorMessage) override;
	virtual void InitGameState() override;
	virtual void StartPlay() override;
	virtual void StartMatch() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void HandleCharacterKilled(class AController* Killer, const TArray<TWeakObjectPtr<AController>>& Assists, ABaseCharacter* Victim, const UItemConfig* DamageCauser = nullptr, bool bWasHeadShot = false);
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	virtual FString InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal) override;
	virtual void RestartPlayer(AController* NewPlayer) override;
	virtual void PreLogin(
		const FString& Options,
		const FString& Address,
		const FUniqueNetIdRepl& UniqueId,
		FString& ErrorMessage) override;
	virtual APlayerController* Login(
		UPlayer* NewPlayer,
		ENetRole InRemoteRole,
		const FString& Portal,
		const FString& Options,
		const FUniqueNetIdRepl& UniqueId,
		FString& ErrorMessage) override;
	virtual void Logout(AController* Exiting) override;
	virtual void ScheduleMatchStart(float DelaySeconds);
	virtual void StartMatchFromCountdown();
	virtual void ResetPlayers();
	virtual ABotAIController* SpawnBot(APlayerSlot* Slot);
	virtual bool CheckAllTeamDead(ETeamId TeamId);
	virtual void AutoBuyForBots();
	virtual void SavePlayersGunsForNextRound();
	AShooterGameState* GetShooterGS() const;
	virtual void RegisterCorpse(AActor* Corpse);
	virtual void CleanupCorpses();
	virtual EMatchMode GetMatchMode() const {
		return EMatchMode::None;
	}
	virtual bool IsDamageAllowed(AController* Killer, AController* Victim) const;

	FOnCharacterDead OnCharacterDead;
protected:
    virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void HandleMatchHasStarted() override;
	virtual APawn* SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform) override;

	virtual void StartRound();
	virtual void EndRound(ETeamId WinningTeam);
	virtual void EndGame(ETeamId WinningTeam);
	void TravelToLobby();

	FTimerHandle RoundStartTimer;
	bool bRoundInProgress = false;
	TUniquePtr<BotStateManager> BotManager;

	UPROPERTY()
	TArray<TWeakObjectPtr<AActor>> Corpses;

	bool bRoundStarted = false;
	int RoomId = 0;

	FTimerHandle StartRoundTimerHandle;
	UFUNCTION()
	void StartRoundDelayed();

	virtual FTransform GetSpawnTransformForSlot(const APlayerSlot& Slot);
	AShooterGameState* CachedGS;

	FTimerHandle MatchStartCountdownHandle;
	float MatchStartDelayDefault = 15.f;
	float MatchStartDelayWhenAllJoined = 3.f;
};
