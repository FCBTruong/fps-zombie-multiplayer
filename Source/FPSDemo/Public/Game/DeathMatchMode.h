// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Game/ShooterGameMode.h"
#include "DeathMatchMode.generated.h"

/**
 * 
 */
UCLASS()
class FPSDEMO_API ADeathMatchMode : public AShooterGameMode
{
	GENERATED_BODY()

protected:
	virtual void StartPlay() override;
	virtual void RestartPlayer(AController* NewPlayer) override;
	virtual void OnCharacterKilled(class AController* Killer, ABaseCharacter* Victim, const UItemConfig* DamageCauser, bool bWasHeadShot) override;
private:
	void OnRoundTimeExpired();
	FTimerHandle RoundTimerHandle;

	static constexpr int32 TimePerRound = 60 * 3; // seconds
};
