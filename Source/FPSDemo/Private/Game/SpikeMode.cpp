// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/SpikeMode.h"
#include "Game/ShooterGameState.h"
#include <GameFramework/PlayerStart.h>
#include "Game/ActorManager.h"
#include <Kismet/GameplayStatics.h>
#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/BaseCharacter.h"
#include "Bot/BotRole.h"
#include "Engine/TriggerBox.h"
#include "Engine/TargetPoint.h"

void ASpikeMode::StartPlay()
{
	UE_LOG(LogTemp, Warning, TEXT("SpikeMode Game Started!"));
	Super::StartPlay();
	
	// decided which team is attacker/defender
	AShooterGameState* GS = GetGameState<AShooterGameState>();
	FName AttackerTeam = (FMath::RandBool()) ? FName("A") : FName("B");
	AttackerTeam = "A"; // for testing
	GS->SetAttackerTeam(AttackerTeam);
	/*SpawnBot("B");
	SpawnBot("B");*/
	//SpawnBot("B");
	SpawnBot("B");
	SpawnBot("A");
	//SpawnBot("A");
	//SpawnBot("A");

	StartRound();
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

	// update blackboard for bots
	// After spike planted, we should update all bot's role to be suitable for game state

	const EBotRole AttackerRoles[] =
	{
		EBotRole::Holder,
		EBotRole::Scout
	};
	const EBotRole DefenderRoles[] =
	{
		EBotRole::Scout
	};
	TArray<ABotAIController*> Defenders;
	for (ABotAIController* Bot : BotControllers)
	{
		UBlackboardComponent* BB = Bot->GetBlackboardComponent();
		if (!BB) continue;

		AMyPlayerState* PS = Bot->GetPlayerState<AMyPlayerState>();
		if (PS->IsAlive() == false) continue;
		
		bool IsAttacker = (PS->GetTeamID() == GS->GetAttackerTeam());
		if (IsAttacker) {
			const int32 Index = FMath::RandRange(0, UE_ARRAY_COUNT(AttackerRoles) - 1);
			const EBotRole ChosenRole = AttackerRoles[Index];
			BB->SetValueAsEnum(TEXT("E_Role"), static_cast<uint8>(ChosenRole));
		}
		else {
			const int32 Index = FMath::RandRange(0, UE_ARRAY_COUNT(DefenderRoles) - 1);
			const EBotRole ChosenRole = DefenderRoles[Index];
			BB->SetValueAsEnum(TEXT("E_Role"), static_cast<uint8>(ChosenRole));
			Defenders.Add(Bot);
		}
		BB->SetValueAsVector(TEXT("Vec_SpikeLocation"), Location);
	}
	// assign one defender as spike defuser
	if (Defenders.Num() > 0)
	{
		int32 RandomIndex = FMath::RandRange(0, Defenders.Num() - 1);
		ABotAIController* SpikeDefuserBot = Defenders[RandomIndex];
		SpikeDefuserBot->GetBlackboardComponent()->SetValueAsEnum(TEXT("E_Role"), static_cast<uint8>(EBotRole::D_Defuser));
		UE_LOG(LogTemp, Warning, TEXT("Bot %s assigned as Spike Defuser"), *SpikeDefuserBot->GetName());
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

		for (ABotAIController* Bot : BotControllers)
		{
			UBlackboardComponent* BB = Bot->GetBlackboardComponent();
			if (BB)
			{
				BB->SetValueAsEnum(TEXT("E_Role"), static_cast<uint8>(EBotRole::Scout));
			}
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
			//CleanPawnsOnMap();
			ResetPlayers();
			StartRound();
		},
		5.0f,
		false
	);
}

void ASpikeMode::StartRound()
{
	AShooterGameState* GS = GetGameState<AShooterGameState>();

	AActorManager::Get(GetWorld())->ResetPlayerStartsUsage();

	TArray<ABotAIController*> Attackers;

	FName BombSite = FMath::RandBool() ? FName(TEXT("A")) : FName(TEXT("B"));
	FVector PlantLocation =
		(BombSite == FName(TEXT("A")))
		? AActorManager::Get(GetWorld())->GetAreaBombA()->GetActorLocation()
		: AActorManager::Get(GetWorld())->GetAreaBombB()->GetActorLocation();

	const EBotRole AttackerRoles[] =
	{
		//EBotRole::A_Escort,
		EBotRole::Scout
	};
	const EBotRole DefenderRoles[] =
	{
	/*	EBotRole::Holder,*/
		EBotRole::Scout
	};
	for (ABotAIController* Bot : BotControllers)
	{
		Bot->ResetAIState();
		AMyPlayerState* PS = Bot->GetPlayerState<AMyPlayerState>();

		bool IsAttacker = (PS->GetTeamID() == GS->GetAttackerTeam());
		UE_LOG(LogTemp, Warning, TEXT("Deubg: Bot %s is %s"), *Bot->GetName(), IsAttacker ? TEXT("Attacker") : TEXT("Defender"));
		// update bot data to blackboard
		UBlackboardComponent* BB = Bot->GetBlackboardComponent();
		if (!BB) continue;

		UE_LOG(LogTemp, Warning, TEXT("Debug: Updating bot %s blackboard"), *Bot->GetName());

		BB->SetValueAsBool(TEXT("B_IsAttacker"), IsAttacker);
		BB->SetValueAsName(TEXT("Name_BombSite"), BombSite);

		BB->SetValueAsVector(TEXT("Vec_PlantLocation"), PlantLocation);

		if (IsAttacker)
		{
			const int32 Index = FMath::RandRange(0, UE_ARRAY_COUNT(AttackerRoles) - 1);
			const EBotRole ChosenRole = AttackerRoles[Index];
			BB->SetValueAsEnum(TEXT("E_Role"), static_cast<uint8>(ChosenRole));
			Attackers.Add(Bot);
		}
		else {
			const int32 Index = FMath::RandRange(0, UE_ARRAY_COUNT(DefenderRoles) - 1);
			const EBotRole ChosenRole = DefenderRoles[Index];
			BB->SetValueAsEnum(TEXT("E_Role"), static_cast<uint8>(ChosenRole));
		}
	}
	// auto assign spike carrier for one of the attackers
	if (Attackers.Num() > 0)
	{
		int32 RandomIndex = FMath::RandRange(0, Attackers.Num() - 1);
		ABotAIController* SpikeCarrierBot = Attackers[RandomIndex];
		SpikeCarrierBot->GetBlackboardComponent()->SetValueAsEnum(TEXT("E_Role"), static_cast<uint8>(EBotRole::A_FindSpike));
		UE_LOG(LogTemp, Warning, TEXT("Bot %s assigned as Spike Carrier"), *SpikeCarrierBot->GetName());
	}

	UE_LOG(LogTemp, Warning, TEXT("Starting new round..."));

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

	GMR->CreatePickupActor(P);
	UE_LOG(LogTemp, Warning, TEXT("Object address = %p"), GMR);
	
	// add spike data to map
	
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
		
		if (bHasSpike) {
			BB->SetValueAsEnum("E_Role", static_cast<uint8>(EBotRole::A_Carrier));
		}
		UE_LOG(LogTemp, Warning, TEXT("Bot %s notified as Spike Carrier"), *BotController->GetName());
	}
	// re access role for defender bots  
	if (bHasSpike) {
		for (ABotAIController* OtherBot : BotControllers)
		{
			UBlackboardComponent* BB = OtherBot->GetBlackboardComponent();
			EBotRole BotRole = static_cast<EBotRole>(BB->GetValueAsEnum("E_Role"));
			if (BotRole == EBotRole::A_FindSpike) { // no need to find spike anymore			
				BB->SetValueAsEnum("E_Role", static_cast<uint8>(EBotRole::Scout));
			}
		}
	}
	else {
		// must select new carrier if spike dropped
		TArray<ABotAIController*> Attackers;

		for (ABotAIController* Bot : BotControllers)
		{
			AMyPlayerState* PS = Bot->GetPlayerState<AMyPlayerState>();
			if (PS->IsAlive() == false) continue;
			AShooterGameState* GS = GetGameState<AShooterGameState>();
			if (PS->GetTeamID() == GS->GetAttackerTeam())
			{
				Attackers.Add(Bot);
			}
		}

		if (Attackers.Num() > 0)
		{
			int32 RandomIndex = FMath::RandRange(0, Attackers.Num() - 1);
			ABotAIController* SpikeCarrierBot = Attackers[RandomIndex];
			SpikeCarrierBot->GetBlackboardComponent()->SetValueAsEnum(TEXT("E_Role"), static_cast<uint8>(EBotRole::A_FindSpike));
		}
	}
}