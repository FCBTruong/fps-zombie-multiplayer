// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/Modes/Spike/SpikeMode.h"
#include "Game/Framework/ShooterGameState.h"
#include "GameFramework/PlayerStart.h"
#include "Game/Subsystems/ActorManager.h"
#include "Kismet/GameplayStatics.h"
#include "Game/Characters/BaseCharacter.h"
#include "Engine/TargetPoint.h"
#include "Game/AI/BotStateManager.h"
#include "Game/Items/Pickup/PickupItem.h"
#include "Shared/System/ItemsManager.h"
#include "Shared/Data/Items/FirearmConfig.h"
#include "Shared/Data/Items/ItemConfig.h"
#include "Game/Characters/Components/InventoryComponent.h"
#include "Game/Characters/Components/EquipComponent.h"
#include "Game/GameManager.h"
#include "Game/Framework/PlayerSlot.h"
#include "Game/AI/BotAIController.h"
#include "Game/Modes/Spike/Spike.h"

void ASpikeMode::StartPlay()
{
	UE_LOG(LogTemp, Warning, TEXT("SpikeMode Game Started!"));
	Super::StartPlay();

	AActorManager* ActorGMR = AActorManager::Get(GetWorld());
	check(ActorGMR);
	ActorGMR->OnNewPickupItemSpawned.AddUObject(
		this,
		&ASpikeMode::HandleNewPickupItemSpawned
	);
}

void ASpikeMode::PlantSpike(FVector Location, AController* Planter)
{
	if (!Planter) {
		return;
	}

	GetWorld()->GetTimerManager().ClearTimer(RoundTimerHandle);
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = Planter;
	SpawnParams.Instigator = Planter->GetPawn();
	auto PlantedSpike = GetWorld()->SpawnActor<ASpike>(SpikeClass, Location, FRotator::ZeroRotator, SpawnParams);

	CachedGS->SetPlantedSpike(PlantedSpike);
	CachedGS->SetMatchState(EMyMatchState::SPIKE_PLANTED);

	if (BotManager) {
		BotManager->OnSpikePlanted(PlantedSpike);
	}
}

void ASpikeMode::OnSpikeDefused(AController* Defuser)
{
	if (!Defuser) {
		return;
	}
		
	EndRound(ETeamId::Defender);
	if (BotManager) {
		BotManager->OnSpikeDefused();
	}
}

void ASpikeMode::EndRound(ETeamId WinningTeam)
{
	if (CachedGS->GetMatchState() == EMyMatchState::ROUND_ENDED)
	{
		return; // already ended
	}
	Super::EndRound(WinningTeam);

	GetWorld()->GetTimerManager().ClearTimer(RoundTimerHandle);
	CachedGS->SetMatchState(EMyMatchState::ROUND_ENDED);
	CachedGS->Multicast_RoundResult(WinningTeam);

	CachedGS->AddScoreTeam(WinningTeam, 1);
	if (CachedGS->GetScoreTeam(WinningTeam) >= ScoreToWin) {
		EndGame(WinningTeam);
		return;
	}
	float DelayBeforeNewRound = 4.0f;
	bool bShouldSwapSides = (CachedGS->GetCurrentRound() == RoundToSwapSides);

	GetWorld()->GetTimerManager().ClearTimer(StartRoundTimerHandle);
	if (bShouldSwapSides) {
		GetWorld()->GetTimerManager().ClearTimer(SwitchSideTimerHandle);
		DelayBeforeNewRound += 2.0f; // extra delay for side swap
		GetWorld()->GetTimerManager().SetTimer(
			SwitchSideTimerHandle,
			[this]()
			{
				CachedGS->Multicast_SwitchSide();
			},
			2.0f,
			false
		);
	}
	
	GetWorld()->GetTimerManager().SetTimer(
		StartRoundTimerHandle,
		[this, bShouldSwapSides]()
		{
			if (bShouldSwapSides) {
				SwapTeams();
			}
			StartRound();
		},
		DelayBeforeNewRound,
		false
	);
}

void ASpikeMode::StartRound()
{
	Super::StartRound();

	CachedGS->Multicast_RoundStart();
	float Now = 0;
	if (const UWorld* World = GetWorld())
	{
		if (const AGameStateBase* GS = World->GetGameState())
		{
			Now = GS->GetServerWorldTimeSeconds();
		}
	}
	int32 TimeEnd = Now + TimePerRound;
	CachedGS->SetMatchState(EMyMatchState::ROUND_IN_PROGRESS);
	CachedGS->SetRoundEndTime(TimeEnd);

	RestartAllPlayers();
	// remove spike if planted
	if (CachedGS->GetPlantedSpike()) {
		CachedGS->GetPlantedSpike()->Destroy();
		CachedGS->SetPlantedSpike(nullptr);
	}

	// Random spike for attacker team
	AActorManager* ActorMgr = AActorManager::Get(GetWorld());
	check(ActorMgr);
	const FVector SpawnLocation = ActorMgr->GetSpikeStartLocation();
	ActorMgr->CleanPickupItemsOnMap();

	FPickupData P;
	P.Id = ActorMgr->GetNextItemOnMapId();
	P.ItemId = EItemId::SPIKE;
	P.Amount = 1;
	P.Location = SpawnLocation;

	APickupItem* SpikeActor = ActorMgr->CreatePickupActor(P);

	if (!SpikeActor) {
		UE_LOG(LogTemp, Warning, TEXT("Failed to spawn Spike pickup item"));
		return;
	}

	SpikeActor->SetIsActive(true);
	
	if (BotManager) {
		BotManager->OnStartRound(SpikeActor);
	}
	
	GenerateInitialWeapons();
	// auto buy for bots
	AutoBuyForBots();

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

void ASpikeMode::OnSpikeExploded()
{
	UE_LOG(LogTemp, Warning, TEXT("Spike exploded!"));
	EndRound(ETeamId::Attacker);
}

AActor* ASpikeMode::ChoosePlayerStart_Implementation(AController* Player)
{
	if (!Player) {
		return Super::ChoosePlayerStart_Implementation(Player); // fallback
	}
	AMyPlayerState* PS = Player->GetPlayerState<AMyPlayerState>();
	if (!PS) {
		return Super::ChoosePlayerStart_Implementation(Player); // fallback
	}
	AActorManager* AM = AActorManager::Get(GetWorld());
	check(AM);

	ETeamId TeamId = PS->GetTeamId();
	if (TeamId == ETeamId::Attacker)
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

void ASpikeMode::HandleCharacterKilled(const FCharacterKilledEvent& Event)
{
	Super::HandleCharacterKilled(Event);

	ABaseCharacter* VictimPawn = Event.Victim;
	OnCharacterDead.Broadcast(VictimPawn);
	RegisterCorpse(VictimPawn);
	VictimPawn->ApplyRealDeath(/*bDropInventory=*/true);

	AMyPlayerState* VictimPS = VictimPawn->GetPlayerState<AMyPlayerState>();
	if (!VictimPS) {
		UE_LOG(LogTemp, Warning, TEXT("VictimPS is null in NotifyPlayerKilled"));
		return;
	}
	ETeamId VictimTeam = VictimPS->GetTeamId();
	HandlePlayerDeath(VictimPawn->GetController());

	// check if all team dead
	bool IsAllTeamDead = CheckAllTeamDead(VictimTeam);

	if (CachedGS->GetMatchState() != EMyMatchState::ROUND_IN_PROGRESS &&
		CachedGS->GetMatchState() != EMyMatchState::SPIKE_PLANTED)
	{
		return; // do nothing if round not in progress
	}

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
	// If spike NOT planted -> defenders win
	if (!IsSpikePlanted())
	{
		EndRound(ETeamId::Defender);
	}
}

void ASpikeMode::NotifySpikePickedUp(ABaseCharacter* Player)
{
	if (BotManager) {
		BotManager->OnSpikePickedUp(Player);
	}
}

void ASpikeMode::SwapTeams() // switch side
{
	for (APlayerSlot* Slot : CachedGS->Slots) {
		ETeamId CurrentTeam = Slot->GetTeamId();
		ETeamId NewTeam = (CurrentTeam == ETeamId::Attacker) ? ETeamId::Defender : ETeamId::Attacker;
		Slot->SetTeamId(NewTeam);
		Slot->ForceNetUpdate();

		// update skin 
		if (NewTeam == ETeamId::Attacker) {
			Slot->SetCharacterSkin(FGameConstants::SKIN_CHARACTER_ATTACKER);
		}
		else {
			Slot->SetCharacterSkin(FGameConstants::SKIN_CHARACTER_DEFENDER);
		}
	}

	// swap scores too
	int ScoreA = CachedGS->GetTeamAScore();
	int ScoreB = CachedGS->GetTeamBScore();
	CachedGS->SetTeamAScore(ScoreB);
	CachedGS->SetTeamBScore(ScoreA);
}

void ASpikeMode::GenerateInitialWeapons()
{
	AActorManager* AM = AActorManager::Get(GetWorld());
	check(AM);
	auto ItemsManager = UItemsManager::Get(GetWorld());

	// Weapon list
	TArray<EItemId> Items = {
		EItemId::RIFLE_AK_47,
		EItemId::RIFLE_RUSSIAN_AS_VAL,
		EItemId::RIFLE_M16A,
		EItemId::RIFLE_QBZ,
		EItemId::SNIPER_BOLT_R,
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

		for (int32 i = 0; i < Count; ++i)
		{
			FPickupData P;
			P.Id = AM->GetNextItemOnMapId();
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

			APickupItem* PickupActor = AM->CreatePickupActor(P);
			PickupActor->SetIsActive(true);
		}
	}
}

bool ASpikeMode::IsSpikePlanted() const
{
	return CachedGS->GetMatchState() == EMyMatchState::SPIKE_PLANTED;
}

void ASpikeMode::AutoBuyForBots()
{
	if (!BotManager) return;
	const TArray<ABotAIController*>& Bots = BotManager->GetManagedBots();

	TArray<EItemId> InitialWeapons = {
		EItemId::RIFLE_AK_47,
		EItemId::RIFLE_RUSSIAN_AS_VAL,
		EItemId::RIFLE_M16A,
		EItemId::RIFLE_QBZ
	};
	for (ABotAIController* Bot : Bots)
	{
		if (!Bot) continue;
		ABaseCharacter* BotChar = Cast<ABaseCharacter>(Bot->GetPawn());
		if (!BotChar) continue;
		UInventoryComponent* InventoryComp = BotChar->GetInventoryComponent();
		if (!InventoryComp) continue;

		InventoryComp->AddItemFromShop(FPickupData{ 
			.Id = -1, 
			.ItemId = InitialWeapons[FMath::RandRange(0, InitialWeapons.Num() - 1)],
			.Amount = 1,
			.AmmoInClip = 30,
			.AmmoReserve = 90
		});

		UEquipComponent* EquipComp = BotChar->GetEquipComponent();

		if (EquipComp)
		{
			EquipComp->AutoSelectBestWeapon();
		}
	}
}

void ASpikeMode::HandlePlayerDeath(AController* DeadController)
{
	if (!DeadController) return;
	AMyPlayerState* PS = DeadController->GetPlayerState<AMyPlayerState>();
	if (PS)
	{
		PS->SetIsSpectator(true);
	}
}

void ASpikeMode::HandleNewPickupItemSpawned(APickupItem* NewPickupItem)
{
	if (!NewPickupItem) return;
	if (NewPickupItem->GetData().ItemId != EItemId::SPIKE) return;
	if (BotManager) {
		BotManager->OnSpikeDropped(NewPickupItem);
	}
}

void ASpikeMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (AActorManager* ActorGMR = AActorManager::Get(GetWorld()))
	{
		ActorGMR->OnNewPickupItemSpawned.RemoveAll(this);
	}
	Super::EndPlay(EndPlayReason);
}
