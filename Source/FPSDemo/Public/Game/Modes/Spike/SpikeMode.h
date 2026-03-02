// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Game/Framework/ShooterGameMode.h"
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

public:
	void PlantSpike(FVector Location, AController* Planter);
	bool IsSpikePlanted() const;
	void OnSpikeDefused(AController* Defuser);
	void NotifySpikePickedUp(ABaseCharacter* Player);
	void OnSpikeExploded();
	EMatchMode GetMatchMode() const final { return EMatchMode::Spike; }

	static constexpr int32 ScoreToWin = 3; // good is 7
	static constexpr int32 RoundToSwapSides = ScoreToWin - 1;
	static constexpr int32 TimePerRound = 90; // seconds

protected:
	virtual void StartPlay() override;
	virtual void StartRound() override;
	virtual void EndRound(ETeamId WinningTeam) override;
	virtual void EndGame(ETeamId WinningTeam) override;
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
	virtual void HandleCharacterKilled(AController* Killer, const TArray<TWeakObjectPtr<AController>>& Assists,
		ABaseCharacter* VictimPawn, const UItemConfig* DamageCauser, bool bWasHeadShot) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(EditDefaultsOnly, Category = "Weapons")
	TSubclassOf<ASpike> SpikeClass;

private:
	void AutoBuyForBots();
	void SwapTeams();
	void GenerateInitialWeapons();
	void HandlePlayerDeath(AController* DeadController);
	void HandleNewPickupItemSpawned(APickupItem* NewPickupItem);
	UFUNCTION()
	void OnRoundTimeExpired();

	FTimerHandle SwitchSideTimerHandle;
	FTimerHandle RoundTimerHandle;
};
