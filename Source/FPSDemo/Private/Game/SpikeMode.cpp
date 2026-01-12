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
	
	SpawnBot();
	SpawnBot();
	SpawnBot();

	// get all controllers and assign teams
	AShooterGameState* GS = GetGameState<AShooterGameState>();
	for (APlayerState* PS : GS->PlayerArray)
	{
		AMyPlayerState* MyPS = Cast<AMyPlayerState>(PS);
		if (!MyPS) continue;
		AController* PC = MyPS->GetOwner<AController>();
		if (!PC) continue;
		AssignPlayerTeamInit(PC);
	}
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
		BotManager->OnSpikePlanted(PlantedSpike);
	}
}

void ASpikeMode::DefuseSpike(AController* Defuser)
{
	if (PlantedSpike)
	{
		UE_LOG(LogTemp, Warning, TEXT("Spike defused by %s"), *Defuser->GetName());
		
		EndRound(ETeamId::Defender);

		if (BotManager) {
			BotManager->OnSpikeDefused();
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("No spike to defuse"));
	}
}

void ASpikeMode::EndRound(ETeamId WinningTeam)
{
	Super::EndRound(WinningTeam);
	
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
	Super::StartRound();
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
		BotManager->OnStartRound(PickupActor);
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

void ASpikeMode::EndGame(ETeamId WinningTeam)
{
	AShooterGameState* GS = GetGameState<AShooterGameState>();
	if (GS) {
		GS->SetMatchState(EMyMatchState::GAME_ENDED);
		//GS->Multicast_GameResult(WinningTeam);
	}
}

void ASpikeMode::SpikeExploded()
{
	UE_LOG(LogTemp, Warning, TEXT("Spike exploded!"));
	EndRound(ETeamId::Attacker);
}

AActor* ASpikeMode::ChoosePlayerStart_Implementation(AController* Player)
{
	UE_LOG(LogTemp, Warning, TEXT("DEBUGXX: ChoosePlayerStart_Implementation..."));
	AMyPlayerState* PS = Player->GetPlayerState<AMyPlayerState>();

	if (!PS) {
		UE_LOG(LogTemp, Warning, TEXT("PlayerState is null in ChoosePlayerStart_Implementation"));
		return Super::ChoosePlayerStart_Implementation(Player); // fallback
	}


	ETeamId TeamId = PS->GetTeamId();

	AShooterGameState* GS = GetGameState<AShooterGameState>();
	AActorManager* AM = AActorManager::Get(GetWorld());

	if (TeamId == ETeamId::Attacker)
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


void ASpikeMode::OnCharacterKilled(class AController* Killer, ABaseCharacter* VictimPawn, const UItemConfig* DamageCauser, bool bWasHeatShot)
{
	Super::OnCharacterKilled(Killer, VictimPawn, DamageCauser, bWasHeatShot);

	RegisterCorpse(VictimPawn);
	VictimPawn->ApplyRealDeath(/*bDropInventory=*/true);

	AMyPlayerState* VictimPS = VictimPawn ? VictimPawn->GetPlayerState<AMyPlayerState>() : nullptr;
	if (!VictimPS) {
		UE_LOG(LogTemp, Warning, TEXT("VictimPS is null in NotifyPlayerKilled"));
		return;
	}
	ETeamId VictimTeam = VictimPS->GetTeamId();

	// check if spike carrier is killed
	

	// check if all team dead
	bool IsAllTeamDead = CheckAllTeamDead(VictimTeam);

	AShooterGameState* GS = GetGameState<AShooterGameState>();
	if (IsAllTeamDead)
	{
		ETeamId WinningTeam;
		// if team dead if counter -> end the game with attacker win
		if (VictimTeam == ETeamId::Defender)
		{
			WinningTeam = ETeamId::Attacker;
			EndRound(WinningTeam);
		}
		else 
		{
			// check the bomb status
			if (!IsSpikePlanted())
			{
				WinningTeam = ETeamId::Defender;
				EndRound(WinningTeam);
			}
		}
	}

	TWeakObjectPtr<AController> VictimCtrlWeak = VictimPawn->Controller;

	const float SpectateDelay = 2.f;
	FTimerDelegate SpectateDel;

	MoveSpectatorsOffDeadPawn(VictimPawn);
	if (AMyPlayerController* PC = Cast<AMyPlayerController>(VictimCtrlWeak.Get()))
	{
		PC->SetPlayerSpectate();
	}
	SpectateDel.BindLambda([VictimCtrlWeak]()
		{
			if (!VictimCtrlWeak.IsValid()) return;

			if (AMyPlayerController* PC = Cast<AMyPlayerController>(VictimCtrlWeak.Get()))
			{
				PC->RequestSpectateNextPlayer();
			}
		});

	FTimerHandle SpectateHandle;
	GetWorldTimerManager().SetTimer(SpectateHandle, SpectateDel, SpectateDelay, false);
}


void ASpikeMode::OnRoundTimeExpired()
{
	AShooterGameState* GS = GetGameState<AShooterGameState>();

	// If spike NOT planted -> defenders win
	if (!IsSpikePlanted())
	{
		EndRound(ETeamId::Defender);
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

void ASpikeMode::AssignPlayerTeamInit(AController* NewPlayer)
{
	UE_LOG(LogTemp, Warning, TEXT("AddPlayer called in AShooterGameMode"));
	if (!NewPlayer)
	{
		return;
	}

	AMyPlayerState* PS = NewPlayer->GetPlayerState<AMyPlayerState>();
	if (!PS) {
		UE_LOG(LogTemp, Warning, TEXT("PlayerState is null in AddPlayer"));
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("Adding player to team..."));

	ETeamId AssignedTeam;

	AShooterGameState* GS = GetGameState<AShooterGameState>();
	if (!GS) {
		UE_LOG(LogTemp, Warning, TEXT("GameState is null in AssignPlayerTeam"));
		return;
	}

	// pick suitable team
	int32 AttackerCount = 0;
	int32 DefenderCount = 0;
	TArray<APlayerState*> PlayerStates = GS->PlayerArray;
	for (APlayerState* ExistingPS : PlayerStates)
	{
		if (!ExistingPS) continue;
		AMyPlayerState* MyExistingPS = Cast<AMyPlayerState>(ExistingPS);
		if (MyExistingPS && MyExistingPS->GetTeamId() == ETeamId::Attacker)
		{
			AttackerCount++;
		}
		else if (MyExistingPS && MyExistingPS->GetTeamId() == ETeamId::Defender)
		{
			DefenderCount++;
		}
	}
	if (AttackerCount <= DefenderCount)
	{
		AssignedTeam = ETeamId::Attacker;
	}
	else
	{
		AssignedTeam = ETeamId::Defender;
	}

	PS->SetTeamId(AssignedTeam);
}