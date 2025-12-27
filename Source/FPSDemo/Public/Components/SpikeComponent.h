// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SpikeComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FPSDEMO_API USpikeComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	USpikeComponent();

public:
	void RequestPlantSpike();
	void RequestStopPlantSpike();

private:
	FTimerHandle PlantTimerHandle;

	UFUNCTION(Server, Reliable)
	void ServerStartPlantSpike();

	UFUNCTION(Server, Reliable)
	void ServerStopPlantSpike();

	void FinishPlantSpike();

	bool CanPlantHere() const;
	void StartPlant_Internal();
};
