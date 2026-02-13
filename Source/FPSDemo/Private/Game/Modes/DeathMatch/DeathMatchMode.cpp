// Fill out your copyright notice in the Description page of Project Settings.
#include "Game/Modes/DeathMatch/DeathMatchMode.h"
#include "Game/Subsystems/ActorManager.h"

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
	RestartAllPlayers();
}

void ADeathMatchMode::OnRoundTimeExpired()
{
	UE_LOG(LogTemp, Warning, TEXT("ADeathMatchMode: Round Time Expired!"));
	// End the round with no winning team
	EndGame(ETeamId::None);
}

void ADeathMatchMode::HandleCharacterKilled(AController* Killer, const TArray<TWeakObjectPtr<AController>>& Assists, ABaseCharacter* VictimPawn, const UItemConfig* DamageCauser, bool bWasHeadShot)
{
	Super::HandleCharacterKilled(Killer, Assists, VictimPawn, DamageCauser, bWasHeadShot);

	// restart victim after 3 seconds
	VictimPawn->ApplyRealDeath(/*bDropInventory=*/false);
	FTimerHandle RespawnTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(
		RespawnTimerHandle,
		[ this, VictimPawn]()
		{
			if (VictimPawn && VictimPawn->GetController())
			{
				RestartPlayer(VictimPawn->GetController());
			}
		},
		3.0f,
		false
	);
}

void ADeathMatchMode::RestartPlayer(AController* Controller)
{
	if (!Controller) return;
	if (Controller->GetPawn()) {
		Controller->GetPawn()->Destroy();
	}
	Controller->StartSpot = nullptr; // clear start spot to force choosing a new one
	Controller->UnPossess();
	Super::RestartPlayer(Controller);


	APawn* Pawn = Controller->GetPawn();
	if (!Pawn) return;
	AActorManager* AM = AActorManager::Get(GetWorld());

	const FVector RandomLoc = AM->RandomLocationOnMap();
	const FRotator RandomRot = FRotator(0.f, FMath::FRandRange(0.f, 360.f), 0.f);

	FTransform SpawnTM(RandomRot, RandomLoc);

	Pawn->TeleportTo(RandomLoc, RandomRot, false, true);
	Pawn->ForceNetUpdate();
}



