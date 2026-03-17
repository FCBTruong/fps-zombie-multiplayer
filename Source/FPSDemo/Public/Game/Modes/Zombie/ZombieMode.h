// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Game/Framework/ShooterGameMode.h"
#include "ZombieMode.generated.h"

/**
 * 
 */
UCLASS()
class FPSDEMO_API AZombieMode : public AShooterGameMode
{
	GENERATED_BODY()

public:
	virtual void HandleCharacterKilled(const FCharacterKilledEvent& Event) override;
	void BecomeHero(ABaseCharacter* Character);
	EMatchMode GetMatchMode() const final { return EMatchMode::Zombie; }

protected:
	virtual void StartPlay() override;
	virtual void StartRound() override;
	virtual void EndRound(ETeamId WinningTeam) override;
	virtual void EndGame(ETeamId WinningTeam) override;
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;

private:
	void RandomZombie();
	void EnterFightState();
	void BecomeZombie(ABaseCharacter* Char);
	void ReviveZombie(ABaseCharacter* ZombieCharacter);
	void HandleHumanKilled(ABaseCharacter* VictimPawn);
	void HandleZombieKilled(ABaseCharacter* VictimPawn, const UItemConfig* DamageCauser);
	void HandleHeroKilled(ABaseCharacter* VictimPawn);
	void HandlePermanentZombieDeath(ABaseCharacter* VictimPawn);
	void ScheduleZombieRevive(ABaseCharacter* VictimPawn);
	void OnRoundTimeExpired();
	void StartSpectating(ABaseCharacter* VictimPawn);
	void SpawnAirdropCrate();
	void SpawnHealPack();
	void CheckAndSpawnAirdropCrate();
	void CheckAndSpawnHealPack();
	ABaseCharacter* ChooseZombie() const;
	bool ShouldEnterHeroPhase(int32 TotalPlayers, int32 AliveSoldiers) const;

	FTimerHandle BuyingTimerHandle;
	FTimerHandle FightStateTimerHandle;
	FTimerHandle AirdropCheckTimer;
	FTimerHandle HealPackCheckTimer;

	const int RoundProgressTime = 120; // seconds - 2 minutes
	const float DelayBeforeNewRound = 3.f;
};
