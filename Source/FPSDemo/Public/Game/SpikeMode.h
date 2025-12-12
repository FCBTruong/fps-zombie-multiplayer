// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Game/ShooterGameMode.h"
#include "Controllers/MyPlayerController.h"
#include "Spike/Spike.h"
#include "SpikeMode.generated.h"

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
	virtual void NotifyPlayerKilled(class AController* Killer, class AController* Victim, class UWeaponData* DamageCauser, bool bWasHeadShot) override;
public:
	virtual void StartPlay() override;
	void StartRound();
	void EndRound(FName WinningTeam);
	void EndGame(FName WinningTeam);
	void PlantSpike(FVector Location, AMyPlayerController* Planter);
	bool IsSpikePlanted() {
		return PlantedSpike != nullptr;
	}
	void DefuseSpike(AMyPlayerController* Defuser);
	void SpikeExploded();
	static constexpr int32 ScoreToWin = 9;
	static constexpr int32 TimePerRound = 90; // seconds
	void NotifyPlayerSpikeState(ABaseCharacter* Player, bool bHasSpike);
};
