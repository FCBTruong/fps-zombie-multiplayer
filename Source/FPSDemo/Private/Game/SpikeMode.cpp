// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/SpikeMode.h"
#include "Game/ShooterGameState.h"
#include <GameFramework/PlayerStart.h>
#include "Game/ActorManager.h"
#include <Kismet/GameplayStatics.h>
#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/BaseCharacter.h"

void ASpikeMode::StartPlay()
{
	UE_LOG(LogTemp, Warning, TEXT("SpikeMode Game Started!"));
	Super::StartPlay();
	
	// decided which team is attacker/defender
	AShooterGameState* GS = GetGameState<AShooterGameState>();
	FName AttackerTeam = (FMath::RandBool()) ? FName("A") : FName("B");
	AttackerTeam = "A"; // for testing
	GS->SetAttackerTeam(AttackerTeam);
	SpawnBot("B");
	SpawnBot("B");
	SpawnBot("B");
	SpawnBot("A");
	StartRound();
}

void ASpikeMode::PlantSpike(FVector Location, AMyPlayerController* Planter)
{
	UE_LOG(LogTemp, Warning, TEXT("Spike planted at location: %s"), *Location.ToString());

	GetWorld()->GetTimerManager().ClearTimer(RoundTimerHandle);
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Planter;
	SpawnParams.Instigator = Planter ? Planter->GetPawn() : nullptr;
	PlantedSpike = GetWorld()->SpawnActor<ASpike>(SpikeClass, Location, FRotator::ZeroRotator, SpawnParams);

	AShooterGameState* GS = GetGameState<AShooterGameState>();
	GS->SetMatchState(EMyMatchState::SPIKE_PLANTED);

	// update blackboard for bots
	for (ABotAIController* Bot : BotControllers)
	{
		UBlackboardComponent* BB = Bot->GetBlackboardComponent();
		if (BB)
		{
			BB->SetValueAsBool(TEXT("B_IsSpikePlanted"), true);

			// random pick hold location for player to move to
			FName BombSite = BB->GetValueAsName(TEXT("Name_BombSite"));
			FVector HoldLocation = AActorManager::Get(GetWorld())->GetRandomHoldLocationNearBombSite(BombSite);
			BB->SetValueAsVector(TEXT("Vec_HoldLocation"), HoldLocation);
		}
	}
}

void ASpikeMode::DefuseSpike(AMyPlayerController* Defuser)
{
	if (PlantedSpike)
	{
		UE_LOG(LogTemp, Warning, TEXT("Spike defused by %s"), *Defuser->GetName());
		PlantedSpike->Defused();
		// check game result
		AShooterGameState* GS = GetGameState<AShooterGameState>();

		
		FName WinningTeam = GS->GetAttackerTeam() == FName("A") ? FName("B") : FName("A");
		EndRound(WinningTeam);
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
	
	// Start next round after delay
	GetWorld()->GetTimerManager().SetTimer(
		StartRoundTimerHandle,
		this,
		&ASpikeMode::StartRound,
		5.0f,     // delay
		false      // non-looping
	);
}

void ASpikeMode::StartRound()
{
	AShooterGameState* GS = GetGameState<AShooterGameState>();

	AActorManager::Get(GetWorld())->ResetPlayerStartsUsage();

	TArray<ABotAIController*> Attackers;

	FName BombSite = FMath::RandBool() ? FName(TEXT("A")) : FName(TEXT("B"));
	for (ABotAIController* Bot : BotControllers)
	{
		Bot->ResetAIState();
		AMyPlayerState* PS = Bot->GetPlayerState<AMyPlayerState>();

		bool IsAttacker = (PS->GetTeamID() == GS->GetAttackerTeam());
		// update bot data to blackboard
		UBlackboardComponent* BB = Bot->GetBlackboardComponent();
		if (!BB) continue;

		BB->SetValueAsBool(TEXT("B_IsAttacker"), IsAttacker);
		BB->SetValueAsName(TEXT("Name_BombSite"), BombSite);

		FVector PlantLocation =
			(BombSite == FName(TEXT("A")))
			? AActorManager::Get(GetWorld())->GetAreaBombA()->GetActorLocation()
			: AActorManager::Get(GetWorld())->GetAreaBombB()->GetActorLocation();
		BB->SetValueAsVector(TEXT("Vec_PlantLocation"), PlantLocation);

		if (IsAttacker)
		{
			Attackers.Add(Bot);
		}
	}
	// auto assign spike carrier for one of the attackers
	if (Attackers.Num() > 0)
	{
		int32 RandomIndex = FMath::RandRange(0, Attackers.Num() - 1);
		ABotAIController* SpikeCarrierBot = Attackers[RandomIndex];
		SpikeCarrierBot->GetBlackboardComponent()->SetValueAsBool("B_IsSpikeCarrier", true);
		UE_LOG(LogTemp, Warning, TEXT("Bot %s assigned as Spike Carrier"), *SpikeCarrierBot->GetName());
		SpikeCarrierBot->GetBlackboardComponent()->SetValueAsVector("Vec_SpikeLocation", AActorManager::Get(GetWorld())->GetSpikeStartLocation());
	}

	UE_LOG(LogTemp, Warning, TEXT("Starting new round..."));

	// Clean map

	if (PlantedSpike)
	{
		PlantedSpike->Destroy();
		PlantedSpike = nullptr;
	}
	// reset players
	ResetPlayers();

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

	GMR->CreatePickupActor(P);
	UE_LOG(LogTemp, Warning, TEXT("Object address = %p"), GMR);
	
	// add spike data to map
	
	GS->SetMatchState(EMyMatchState::ROUND_IN_PROGRESS);

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
	AMyPlayerState* PS = Player->GetPlayerState<AMyPlayerState>();

	if (!PS) {
		UE_LOG(LogTemp, Warning, TEXT("PlayerState is null in ChoosePlayerStart_Implementation"));
		return Super::ChoosePlayerStart_Implementation(Player); // fallback
	}


	FName TeamId = PS->GetTeamID();

	AShooterGameState* GS = GetGameState<AShooterGameState>();

	if (TeamId == GS->GetAttackerTeam())
	{
		APlayerStart* AttackerStart = AActorManager::Get(GetWorld()) ? AActorManager::Get(GetWorld())->GetRandomAttackerStart() : nullptr;
		if (AttackerStart)
		{
			return AttackerStart;
		}
	}
	else {

		APlayerStart* DefenderStart = AActorManager::Get(GetWorld()) ? AActorManager::Get(GetWorld())->GetRandomDefenderStart() : nullptr;
		if (DefenderStart)
		{
			return DefenderStart;
		}
	}
	

	return Super::ChoosePlayerStart_Implementation(Player); // fallback
}


void ASpikeMode::NotifyPlayerKilled(class AController* Killer, class AController* Victim, class UWeaponData* DamageCauser, bool bWasHeatShot)
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

void ASpikeMode::NotifyPlayerSpikeState(ABaseCharacter* Player, bool bHasSpike)
{
	// If bot, set blackboard value
	ABotAIController* BotController = Cast<ABotAIController>(Player->GetController());
	if (BotController)
	{
		UBlackboardComponent* BB = BotController->GetBlackboardComponent();
		if (BB)
		{
			BB->SetValueAsBool("B_HasSpike", bHasSpike);
			UE_LOG(LogTemp, Warning, TEXT("Bot %s notified as Spike Carrier"), *BotController->GetName());
		}

		// update other bots' blackboard
		for (ABotAIController* OtherBot : BotControllers)
		{
			AMyPlayerState* OtherPS = OtherBot->GetPlayerState<AMyPlayerState>();
			if (OtherPS->GetTeamID() != BotController->GetPlayerState<AMyPlayerState>()->GetTeamID()) continue;
			UBlackboardComponent* OtherBB = OtherBot->GetBlackboardComponent();
			if (OtherBB)
			{
				OtherBB->SetValueAsBool("B_TeamHasSpike", bHasSpike);
			}
		}
	}
}