// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Game/ShooterGameMode.h"
#include "ZombieMode.generated.h"

/**
 * 
 */
UCLASS()
class FPSDEMO_API AZombieMode : public AShooterGameMode
{
	GENERATED_BODY()
	
protected:
	virtual void StartPlay() override;
	virtual void StartRound() override;
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
	virtual APawn* SpawnDefaultPawnAtTransform_Implementation(
		AController* NewPlayer,
		const FTransform& SpawnTransform
	) override;
	virtual void EndRound(ETeamId WinningTeam) override;
	virtual void EndGame(ETeamId WinningTeam) override;

	void RandomZombie();
	void EnterFightState();
	void BecomeZombie(AController* Controller);
	void ReviveZombie(ABaseCharacter* ZombieCharacter);
	void HandleHumanKilled(ABaseCharacter* VictimPawn);
	void HandleZombieKilled(ABaseCharacter* VictimPawn, const UItemConfig* DamageCauser);
	void HandleHeroKilled(ABaseCharacter* VictimPawn);
	void HandlePermanentZombieDeath(ABaseCharacter* VictimPawn);
	void ScheduleZombieRevive(ABaseCharacter* VictimPawn);
	void OnRoundTimeExpired();
	void StartSpectating(ABaseCharacter* VictimPawn);

	FTimerHandle BuyingTimerHandle;
	FTimerHandle FightStateTimerHandle;
	FTimerHandle StartRoundTimerHandle;
public:
	virtual void OnCharacterKilled(class AController* Killer, ABaseCharacter* Victim, const UItemConfig* DamageCauser, bool bWasHeadShot) override;
	virtual EMatchMode GetMatchMode() const {
		return EMatchMode::Zombie;
	}
	void BecomeHero(AController* Controller);
};
