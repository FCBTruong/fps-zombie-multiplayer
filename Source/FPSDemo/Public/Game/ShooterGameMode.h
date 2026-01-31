// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "Controllers/BotAIController.h"
#include "Bot/BotStateManager.h"
#include "Lobby/RoomData.h"
#include "ShooterGameMode.generated.h"

class UItemConfig;
class AShooterGameState;
class ABaseCharacter;

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
	virtual void StartPlay() override;
	virtual void StartMatch() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void OnCharacterKilled(class AController* Killer, ABaseCharacter* Victim, const UItemConfig* DamageCauser = nullptr, bool bWasHeadShot = false);
	virtual void AssignPlayerTeamInit(AController* NewPlayer);
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	virtual FString InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal) override;
	virtual void RestartPlayer(AController* NewPlayer) override;
	virtual void ResetPlayers();
	virtual ABotAIController* SpawnBot(bool IsTeamA);
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
	virtual bool ReadyToStartMatch_Implementation() override;
	virtual void HandleMatchHasStarted() override;
	virtual void StartRound();
	virtual void EndRound(ETeamId WinningTeam);
	virtual void EndGame(ETeamId WinningTeam);
	void TravelToLobby();

	FTimerHandle RoundStartTimer;
	bool bRoundInProgress = false;
	TUniquePtr<BotStateManager> BotManager;
	UPROPERTY()
	TArray<APlayerState*> JoinedPlayers;

	UPROPERTY()
	TArray<TWeakObjectPtr<AActor>> Corpses;

	bool bRoundStarted = false;
	int RoomId = 0;
	bool bIsAllPlayersJoined = false;

	FTimerHandle TryStartMatchHandle;
};
