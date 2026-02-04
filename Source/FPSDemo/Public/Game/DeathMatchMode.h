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

public:
	virtual EMatchMode GetMatchMode() const {
		return EMatchMode::DeathMatch;
	}
protected:
	virtual void StartPlay() override;
	virtual void RestartPlayer(AController* Controller) override;
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	virtual void HandleCharacterKilled(AController* Killer, const TArray<TWeakObjectPtr<AController>>& Assists,
		ABaseCharacter* VictimPawn, const UItemConfig* DamageCauser, bool bWasHeatShot) override;
private:
	void OnRoundTimeExpired();
	FTimerHandle RoundTimerHandle;

	static constexpr int32 TimePerRound = 60 * 5; // seconds
};
