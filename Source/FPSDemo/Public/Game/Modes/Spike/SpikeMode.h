// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Game/Framework/ShooterGameMode.h"
#include "Game/Framework/MyPlayerController.h"
#include "Game/Modes/Spike/Spike.h"
#include "SpikeMode.generated.h"

class UItemConfig;
class APickupItem;
/**
 * 
 */
UCLASS()
class FPSDEMO_API ASpikeMode : public AShooterGameMode
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Weapons")
	TSubclassOf<ASpike> SpikeClass;

	FTimerHandle StartRoundTimerHandle;
	FTimerHandle SwitchSideTimerHandle;
	FTimerHandle RoundTimerHandle;
	void OnRoundTimeExpired();
	virtual void AutoBuyForBots() override;
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
	virtual void HandleCharacterKilled(AController* Killer, const TArray<TWeakObjectPtr<AController>>& Assists, 
		ABaseCharacter* VictimPawn, const UItemConfig* DamageCauser, bool bWasHeatShot) override;
	void HandlePlayerDeath(AController* DeadController);
	void HandleNewPickupItemSpawned(APickupItem* NewPickupItem);
public:
	virtual void StartPlay() override;
	virtual void StartRound() override;
	virtual void EndRound(ETeamId WinningTeam) override;
	virtual void EndGame(ETeamId WinningTeam) override;
	void PlantSpike(FVector Location, AController* Planter);
	bool IsSpikePlanted() const;
	void OnSpikeDefused(AController* Defuser);
	void SpikeExploded();
	static constexpr int32 ScoreToWin = 3; // good is 7
	static constexpr int32 RoundToSwapSides = ScoreToWin - 1;
	static constexpr int32 TimePerRound = 90; // seconds
	void NotifySpikePickedUp(ABaseCharacter* Player);

	virtual EMatchMode GetMatchMode() const {
		return EMatchMode::Spike;
	}

private:
	void SwapTeams();
	void GenerateInitialWeapons();
};
