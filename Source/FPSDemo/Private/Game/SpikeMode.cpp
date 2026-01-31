// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/SpikeMode.h"
#include "Game/ShooterGameState.h"
#include <GameFramework/PlayerStart.h>
#include "Game/ActorManager.h"
#include <Kismet/GameplayStatics.h>
#include "Characters/BaseCharacter.h"
#include "Engine/TargetPoint.h"
#include "Bot/BotStateManager.h"
#include "Pickup/PickupItem.h"
#include "Game/ItemsManager.h"
#include "Items/FirearmConfig.h"
#include "Items/ItemConfig.h"

void ASpikeMode::StartPlay()
{
	UE_LOG(LogTemp, Warning, TEXT("SpikeMode Game Started!"));
	Super::StartPlay();

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
	auto PlantedSpike = GetWorld()->SpawnActor<ASpike>(SpikeClass, Location, FRotator::ZeroRotator, SpawnParams);


	AShooterGameState* GS = GetGameState<AShooterGameState>();
	GS->SetPlantedSpike(PlantedSpike);
	GS->SetMatchState(EMyMatchState::SPIKE_PLANTED);

	if (BotManager) {
		BotManager->OnSpikePlanted(PlantedSpike);
	}
}

void ASpikeMode::OnSpikeDefused(AController* Defuser)
{
	AShooterGameState* GS = GetGameState<AShooterGameState>();

	UE_LOG(LogTemp, Warning, TEXT("Spike defused by %s"), *Defuser->GetName());
		
	EndRound(ETeamId::Defender);

	if (BotManager) {
		BotManager->OnSpikeDefused();
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
	if (GS->GetCurrentRound() == RoundToSwapSides) {
		GetWorld()->GetTimerManager().SetTimer(
			SwitchSideTimerHandle,
			[this]()
			{
				SwapTeams();
			},
			2.0f,
			false
		);
	}

	// save guns for next round if player is alive	
	SavePlayersGunsForNextRound();
	
	GetWorld()->GetTimerManager().SetTimer(
		StartRoundTimerHandle,
		[this]()
		{
			StartRound();
		},
		4.0f,
		false
	);
}

void ASpikeMode::StartRound()
{
	Super::StartRound();
	ResetPlayers();
	AShooterGameState* GS = GetShooterGS();

	// remove spike if planted
	if (GS->GetPlantedSpike()) {
		GS->GetPlantedSpike()->Destroy();
		GS->SetPlantedSpike(nullptr);
	}

	UE_LOG(LogTemp, Warning, TEXT("DEBUGXX: Starting new round..."));

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
	PickupActor->SetIsActive(true);
	UE_LOG(LogTemp, Warning, TEXT("Object address = %p"), PickupActor);
	
	if (BotManager) {
		BotManager->OnStartRound(PickupActor);
	}
	

	GenerateInitialWeapons();
	// auto buy for bots
	AutoBuyForBots();

	// start game after buy phase 3 seconds
	AShooterGameState* GSInner = GetGameState<AShooterGameState>();
	int TimeEnd = GetWorld()->GetTimeSeconds() + TimePerRound;
	GSInner->SetMatchState(EMyMatchState::ROUND_IN_PROGRESS);
	GSInner->SetRoundEndTime(TimeEnd);

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
	Super::EndGame(WinningTeam);
}

void ASpikeMode::SpikeExploded()
{
	UE_LOG(LogTemp, Warning, TEXT("Spike exploded!"));
	EndRound(ETeamId::Attacker);
}

AActor* ASpikeMode::ChoosePlayerStart_Implementation(AController* Player)
{
	UE_LOG(LogTemp, Warning, TEXT("Choosing player start for player %s"), *Player->GetName());
	AMyPlayerState* PS = Player->GetPlayerState<AMyPlayerState>();

	if (!PS) {
		UE_LOG(LogTemp, Warning, TEXT("PlayerState is null in ChoosePlayerStart_Implementation"));
		return Super::ChoosePlayerStart_Implementation(Player); // fallback
	}

	ETeamId TeamId = PS->GetTeamId();
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

void ASpikeMode::SwapTeams() // switch side
{
	AShooterGameState* GS = GetGameState<AShooterGameState>();
	if (!GS) {
		UE_LOG(LogTemp, Warning, TEXT("GameState is null in SwapTeams"));
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("Swapping teams..."));
	TArray<APlayerState*> PlayerStates = GS->PlayerArray;
	for (APlayerState* ExistingPS : PlayerStates)
	{
		if (!ExistingPS) continue;
		AMyPlayerState* MyExistingPS = Cast<AMyPlayerState>(ExistingPS);
		if (MyExistingPS)
		{
			ETeamId CurrentTeam = MyExistingPS->GetTeamId();
			ETeamId NewTeam = (CurrentTeam == ETeamId::Attacker) ? ETeamId::Defender : ETeamId::Attacker;
			UE_LOG(LogTemp, Warning, TEXT("Changing player %s from team %d to team %d"), *MyExistingPS->GetName(), static_cast<int32>(CurrentTeam), static_cast<int32>(NewTeam));
			MyExistingPS->SetTeamId(NewTeam);
		}
	}

	GS->Multicast_SwitchSide();
}

void ASpikeMode::GenerateInitialWeapons()
{
	AActorManager* AM = AActorManager::Get(GetWorld());
	UGameManager* GMR = UGameManager::Get(GetWorld());
	if (!AM || !GMR) return;

	// Weapon list
	TArray<EItemId> Items = {
		EItemId::RIFLE_AK_47,
		EItemId::RIFLE_RUSSIAN_AS_VAL,
		EItemId::RIFLE_M16A,
		EItemId::RIFLE_QBZ,
		EItemId::SNIPER_BOLT_R,
		EItemId::GRENADE_FRAG_BASIC,
		EItemId::GRENADE_INCENDIARY,
		EItemId::GRENADE_SMOKE,
		EItemId::GRENADE_STUN
	};

	const float Distance = 120.0f;
	TArray<FVector> Centers;
	Centers.Add(AM->DefenderWeaponInitPos);
	Centers.Add(AM->AttackerWeaponInitPos);
	
	for (const FVector& CenterPos : Centers)
	{
		// Center the row
		int32 Count = Items.Num();
		float StartOffset = -((Count - 1) * Distance) * 0.5f;

		auto ItemsManager = UItemsManager::Get(GetWorld());
		for (int32 i = 0; i < Count; ++i)
		{
			FPickupData P;
			P.Id = GMR->GetNextItemOnMapId();
			P.ItemId = Items[i];
			P.Amount = 1;

			const UItemConfig* ItemConfig = ItemsManager->GetItemById(Items[i]);
			const UFirearmConfig* FirearmConfig = Cast<UFirearmConfig>(ItemConfig);
			if (FirearmConfig)
			{
				P.AmmoInClip = FirearmConfig->MaxAmmoInClip;
				P.AmmoReserve = FirearmConfig->MaxAmmoInClip * 2;
			}


			// Offset along X axis
			P.Location = CenterPos + FVector(0.f, StartOffset + i * Distance, 0.f);

			APickupItem* PickupActor = GMR->CreatePickupActor(P);
			PickupActor->SetIsActive(true);
			UE_LOG(LogTemp, Warning, TEXT("Pickup %d address = %p"), i, PickupActor);
		}
	}
}

bool ASpikeMode::IsSpikePlanted() const
{
	AShooterGameState* GS = GetGameState<AShooterGameState>();
	return GS->GetMatchState() == EMyMatchState::SPIKE_PLANTED;
}
