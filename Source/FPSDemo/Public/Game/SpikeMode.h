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
public:
	virtual void StartPlay() override;
	void PlantSpike(FVector Location, AMyPlayerController* Planter);
	bool IsSpikePlanted() {
		return PlantedSpike != nullptr;
	}
	void DefuseSpike(AMyPlayerController* Defuser);
};
