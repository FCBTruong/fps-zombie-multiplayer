// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Game/Framework/ShooterGameMode.h"
#include "DeathMatchMode.generated.h"

/**
 * 
 */
UCLASS()
class FPSDEMO_API ADeathMatchMode : public AShooterGameMode
{
	GENERATED_BODY()

public:
	virtual void StartPlay() override;
	EMatchMode GetMatchMode() const final { return EMatchMode::DeathMatch; }

protected:
	virtual void RestartPlayer(AController* Controller) override;
	virtual void HandleCharacterKilled(AController* Killer, const TArray<TWeakObjectPtr<AController>>& Assists,
		ABaseCharacter* VictimPawn, const UItemConfig* DamageCauser, bool bWasHeadShot) override;
private:
	void OnRoundTimeExpired();
	FTimerHandle RoundTimerHandle;

	static constexpr int32 TimePerRound = 60 * 3; // seconds
};
