// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/DeathMatchMode.h"
#include "Game/ActorManager.h"

void ADeathMatchMode::StartPlay()
{
	UE_LOG(LogTemp, Warning, TEXT("SpikeMode Game Started!"));
	Super::StartPlay();

	GetWorld()->GetTimerManager().SetTimer(
		RoundTimerHandle,
		this,
		&ADeathMatchMode::OnRoundTimeExpired,
		TimePerRound,
		false
	);

	AShooterGameState* GSInner = GetGameState<AShooterGameState>();
	int TimeEnd = GetWorld()->GetTimeSeconds() + TimePerRound;
	GSInner->SetMatchState(EMyMatchState::ROUND_IN_PROGRESS);
	GSInner->SetRoundEndTime(TimeEnd);

	// reset players
	ResetPlayers();
}

void ADeathMatchMode::OnRoundTimeExpired()
{
	UE_LOG(LogTemp, Warning, TEXT("ADeathMatchMode: Round Time Expired!"));
	// End the round with no winning team
	EndGame(ETeamId::None);
}

void ADeathMatchMode::OnCharacterKilled(class AController* Killer, ABaseCharacter* Victim, const UItemConfig* DamageCauser, bool bWasHeadShot)
{
	Super::OnCharacterKilled(Killer, Victim, DamageCauser, bWasHeadShot);
	
	// restart victim after 3 seconds
	FTimerHandle RespawnTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(
		RespawnTimerHandle,
		[ this, Victim ]()
		{
			if (Victim && Victim->GetController())
			{
				RestartPlayer(Victim->GetController());
			}
		},
		3.0f,
		false
	);
}

void ADeathMatchMode::RestartPlayer(AController* NewPlayer)
{
	if (!NewPlayer)
	{
		return;
	}
	// destroy existing pawn
	if (APawn* ExistingPawn = NewPlayer->GetPawn())
	{
		ExistingPawn->Destroy();
	}

	AActorManager* AM = AActorManager::Get(GetWorld());

	FVector Location = AM->RandomLocationOnMap();
	FRotator Rotation = FRotator::ZeroRotator;
	FTransform SpawnTransform(Rotation, Location);

	APawn* Pawn = SpawnDefaultPawnAtTransform(NewPlayer, SpawnTransform);

	if (Pawn)
	{
		NewPlayer->Possess(Pawn);
	}
}