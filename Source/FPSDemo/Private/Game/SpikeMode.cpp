// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/SpikeMode.h"
#include "Game/ShooterGameState.h"

void ASpikeMode::StartPlay()
{
	UE_LOG(LogTemp, Warning, TEXT("SpikeMode Game Started!"));
	Super::StartPlay();

	// set game state
	AShooterGameState* GS = GetGameState<AShooterGameState>();
	if (GS) {
		GS->SetMatchState(EMyMatchState::PLAYING);
	}
}

void ASpikeMode::PlantSpike(FVector Location, AMyPlayerController* Planter)
{
	UE_LOG(LogTemp, Warning, TEXT("Spike planted at location: %s"), *Location.ToString());
	
	// Gen object Spike
	if (SpikeClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = Planter;
		SpawnParams.Instigator = Planter ? Planter->GetPawn() : nullptr;
		SpawnParams.Name = FName("SpikeBomb");
		PlantedSpike = GetWorld()->SpawnActor<ASpike>(SpikeClass, Location, FRotator::ZeroRotator, SpawnParams);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("SpikeClass is not set in SpikeMode"));
	}
}

void ASpikeMode::DefuseSpike(AMyPlayerController* Defuser)
{
	if (PlantedSpike)
	{
		UE_LOG(LogTemp, Warning, TEXT("Spike defused by %s"), *Defuser->GetName());
		PlantedSpike->Defused();
		PlantedSpike = nullptr;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No spike to defuse"));
	}
}