// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/SpikeMode.h"
#include "Game/ShooterGameState.h"
#include <GameFramework/PlayerStart.h>
#include "Game/ActorManager.h"
#include <Kismet/GameplayStatics.h>
#include "Characters/BaseCharacter.h"
#include "Engine/TargetPoint.h"
#include "Bot/BotStateManager.h"
#include "Characters/BaseCharacter.h"
#include "Pickup/PickupItem.h"

void ASpikeMode::StartPlay()
{
	UE_LOG(LogTemp, Warning, TEXT("SpikeMode Game Started!"));
	Super::StartPlay();
	
	// decided which team is attacker/defender
	AShooterGameState* GS = GetGameState<AShooterGameState>();
	GS->SetMatchMode(EMatchMode::Spike);
	BotManager->SetMatchMode(EMatchMode::Spike);
	FName AttackerTeam = (FMath::RandBool()) ? FName("A") : FName("B");
	AttackerTeam = "A"; // for testing
	GS->SetAttackerTeam(AttackerTeam);
	SpawnBot("B");
	SpawnBot("B");
	SpawnBot("B");
	SpawnBot("B");
	/*SpawnBot("A");*/
	//SpawnBot("A");
	//SpawnBot("A");
}

void ASpikeMode::PlantSpike(FVector Location, AController* Planter)
{
	UE_LOG(LogTemp, Warning, TEXT("Spike planted at location: %s"), *Location.ToString());

	GetWorld()->GetTimerManager().ClearTimer(RoundTimerHandle);
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Planter;
	SpawnParams.Instigator = Planter ? Planter->GetPawn() : nullptr;
	PlantedSpike = GetWorld()->SpawnActor<ASpike>(SpikeClass, Location, FRotator::ZeroRotator, SpawnParams);

	AShooterGameState* GS = GetGameState<AShooterGameState>();
	GS->SetMatchState(EMyMatchState::SPIKE_PLANTED);

	if (BotManager) {
		BotManager->OnSpikePlanted(GS->GetAttackerTeam(), PlantedSpike);
	}
}

void ASpikeMode::DefuseSpike(AController* Defuser)
{
	if (PlantedSpike)
	{
		UE_LOG(LogTemp, Warning, TEXT("Spike defused by %s"), *Defuser->GetName());
		// check game result
		AShooterGameState* GS = GetGameState<AShooterGameState>();

		
		FName WinningTeam = GS->GetAttackerTeam() == FName("A") ? FName("B") : FName("A");
		EndRound(WinningTeam);

		if (BotManager) {
			BotManager->OnSpikeDefused();
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No spike to defuse"));
	}
}

void ASpikeMode::EndRound(FName WinningTeam)
{
	UE_LOG(LogTemp, Warning, TEXT("Round Ended! Team %s wins!"), *WinningTeam.ToString());
	// Clear timer 
	GetWorld()->GetTimerManager().ClearTimer(RoundTimerHandle);

	AShooterGameState* GS = GetGameState<AShooterGameState>();
	
	GS->SetMatchState(EMyMatchState::ROUND_ENDED);
	GS->Multicast_RoundResult(WinningTeam);

	GS->AddScoreTeam(WinningTeam, 1);

	if (GS->GetScoreTeam(WinningTeam) >= ScoreToWin) {
		EndGame(WinningTeam);
		return;
	}

	// save guns for next round if player is alive	
	SavePlayersGunsForNextRound();
	
	GetWorld()->GetTimerManager().SetTimer(
		StartRoundTimerHandle,
		[this]()
		{
			StartRound();
		},
		5.0f,
		false
	);
}

void ASpikeMode::StartRound()
{
	AActorManager* AM = AActorManager::Get(GetWorld());
	AM->ResetPlayerStartsUsage();
	ResetPlayers();
	AShooterGameState* GS = GetShooterGS();

	UE_LOG(LogTemp, Warning, TEXT("DEBUGXX: Starting new round..."));

	// Clean map

	if (PlantedSpike)
	{
		PlantedSpike->Destroy();
		PlantedSpike = nullptr;
	}

	// Random spike for attacker team
	FVector SpawnLocation = FVector::ZeroVector;
	if (AActorManager::Get(GetWorld())) {
		SpawnLocation = AActorManager::Get(GetWorld())->GetSpikeStartLocation();
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("ActorManager instance is null"));
	}

	UGameManager* GMR = UGameManager::Get(GetWorld());
	GMR->CleanPickupItemsOnMap();

	FPickupData P;
	P.Id = GMR->GetNextItemOnMapId();
	P.ItemId = EItemId::SPIKE;
	P.Amount = 1;
	P.Location = SpawnLocation;

	APickupItem* PickupActor = GMR->CreatePickupActor(P);
	UE_LOG(LogTemp, Warning, TEXT("Object address = %p"), PickupActor);
	
	if (BotManager) {
		BotManager->OnStartRound(AM, GS->GetAttackerTeam(), PickupActor);
	}
	
	GS->SetMatchState(EMyMatchState::BUY_PHASE);
	// auto buy for bots
	AutoBuyForBots();

	// start game after buy phase 3 seconds
	GetWorld()->GetTimerManager().SetTimer(
		RoundTimerHandle,
		[this]()
		{
			AShooterGameState* GSInner = GetGameState<AShooterGameState>();
			GSInner->SetMatchState(EMyMatchState::ROUND_IN_PROGRESS);
		},
		3.0f,
		false
	);

	int TimeEnd = GetWorld()->GetTimeSeconds() + TimePerRound;
	GS->SetRoundEndTime(TimeEnd);


	GetWorld()->GetTimerManager().SetTimer(
		RoundTimerHandle,
		this,
		&ASpikeMode::OnRoundTimeExpired,
		TimePerRound,
		false
	);
}

void ASpikeMode::EndGame(FName WinningTeam)
{
	UE_LOG(LogTemp, Warning, TEXT("Game Over! Team %s wins the game!"), *WinningTeam.ToString());
	AShooterGameState* GS = GetGameState<AShooterGameState>();
	if (GS) {
		GS->SetMatchState(EMyMatchState::GAME_ENDED);
		//GS->Multicast_GameResult(WinningTeam);
	}
}

void ASpikeMode::SpikeExploded()
{
	UE_LOG(LogTemp, Warning, TEXT("Spike exploded!"));
	AShooterGameState* GS = GetGameState<AShooterGameState>();
	FName WinningTeam = GS->GetAttackerTeam() == FName("A") ? FName("A") : FName("B");
	EndRound(WinningTeam);
}

AActor* ASpikeMode::ChoosePlayerStart_Implementation(AController* Player)
{
	UE_LOG(LogTemp, Warning, TEXT("DEBUGXX: ChoosePlayerStart_Implementation..."));
	AMyPlayerState* PS = Player->GetPlayerState<AMyPlayerState>();

	if (!PS) {
		UE_LOG(LogTemp, Warning, TEXT("PlayerState is null in ChoosePlayerStart_Implementation"));
		return Super::ChoosePlayerStart_Implementation(Player); // fallback
	}


	FName TeamId = PS->GetTeamID();

	AShooterGameState* GS = GetGameState<AShooterGameState>();
	AActorManager* AM = AActorManager::Get(GetWorld());

	if (TeamId == GS->GetAttackerTeam())
	{
		UE_LOG(LogTemp, Warning, TEXT("Choosing attacker start for player %s"), *Player->GetName());
		APlayerStart* AttackerStart = AM->GetRandomAttackerStart();
		if (AttackerStart)
		{
			return AttackerStart;
		}
	}
	else {

		APlayerStart* DefenderStart = AM->GetRandomDefenderStart();
		if (DefenderStart)
		{
			return DefenderStart;
		}
	}
	

	return Super::ChoosePlayerStart_Implementation(Player); // fallback
}


void ASpikeMode::NotifyPlayerKilled(class AController* Killer, class AController* Victim, const UItemConfig* DamageCauser, bool bWasHeatShot)
{
	Super::NotifyPlayerKilled(Killer, Victim, DamageCauser, bWasHeatShot);

	AMyPlayerState* VictimPS = Victim ? Victim->GetPlayerState<AMyPlayerState>() : nullptr;
	if (!VictimPS) {
		UE_LOG(LogTemp, Warning, TEXT("VictimPS is null in NotifyPlayerKilled"));
		return;
	}
	FName VictimTeam = VictimPS->GetTeamID();

	// check if spike carrier is killed
	

	// check if all team dead
	bool IsAllTeamDead = CheckAllTeamDead(VictimTeam);

	AShooterGameState* GS = GetGameState<AShooterGameState>();
	if (IsAllTeamDead)
	{
		FName WinningTeam;
		// if team dead if counter -> end the game with attacker win
		if (VictimTeam != GS->GetAttackerTeam())
		{
			WinningTeam = GS->GetAttackerTeam();
			EndRound(WinningTeam);
		}
		else 
		{
			// check the bomb status
			if (!IsSpikePlanted())
			{
				WinningTeam = GS->GetDefenderTeam();
				EndRound(WinningTeam);
			}
		}
	}
}


void ASpikeMode::OnRoundTimeExpired()
{
	AShooterGameState* GS = GetGameState<AShooterGameState>();

	// If spike NOT planted -> defenders win
	if (!IsSpikePlanted())
	{
		EndRound(GS->GetDefenderTeam());
	}
}

void ASpikeMode::NotifySpikeDropped(ABaseCharacter* Player)
{
	if (BotManager) {
		BotManager->OnSpikeDropped();
	}
}

void ASpikeMode::NotifySpikePickedUp(ABaseCharacter* Player)
{
	if (BotManager) {
		BotManager->OnSpikePickedUp(Player);
	}
}