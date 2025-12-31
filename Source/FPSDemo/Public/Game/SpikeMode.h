// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Game/ShooterGameMode.h"
#include "Controllers/MyPlayerController.h"
#include "Spike/Spike.h"
#include "SpikeMode.generated.h"

class UItemConfig;

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
	ASpike* PlantedSpike;
	FTimerHandle StartRoundTimerHandle;
	FTimerHandle RoundTimerHandle;
	void OnRoundTimeExpired();
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
	virtual void NotifyPlayerKilled(class AController* Killer, class AController* Victim, const UItemConfig* DamageCauser, bool bWasHeadShot) override;
public:
	virtual void StartPlay() override;
	virtual void StartRound() override;
	void EndRound(FName WinningTeam);
	void EndGame(FName WinningTeam);
	void PlantSpike(FVector Location, AController* Planter);
	bool IsSpikePlanted() {
		return PlantedSpike != nullptr;
	}
	void DefuseSpike(AController* Defuser);
	void SpikeExploded();
	static constexpr int32 ScoreToWin = 9;
	static constexpr int32 TimePerRound = 90; // seconds
	void NotifySpikeDropped(ABaseCharacter* Player);
	void NotifySpikePickedUp(ABaseCharacter* Player);
	
	ASpike* GetPlantedSpike() const {
		return PlantedSpike;
	}
};
