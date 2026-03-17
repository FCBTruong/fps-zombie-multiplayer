// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/Modes/Zombie/ZombieMode.h"
#include "Game/Framework/ShooterGameState.h"
#include "Game/Characters/BaseCharacter.h"
#include "Game/Characters/Components/RoleComponent.h"
#include "Game/Subsystems/ActorManager.h"
#include "Shared/Data/Items/ItemConfig.h"
#include "Game/Modes/Zombie/AirdropCrate.h"
#include "Game/Framework/PlayerSlot.h"
#include "Game/AI/BotAIController.h"
#include "Game/Modes/Zombie/HealthPack.h"

void AZombieMode::StartPlay()
{
	UE_LOG(LogTemp, Warning, TEXT("Zombie Game Started!"));
	Super::StartPlay();
	bAllowFriendlyFire = false;
}

void AZombieMode::StartRound()
{
	Super::StartRound();

	// clean airdrop crates and clear timer
	GetWorldTimerManager().ClearTimer(AirdropCheckTimer);
	GetWorldTimerManager().ClearTimer(HealPackCheckTimer);

	const TArray<AAirdropCrate*>& Airdrops = CachedGS->GetActiveAirdropCrates();
	for (AAirdropCrate* Crate : Airdrops)
	{
		if (Crate && !Crate->IsPendingKillPending())
		{
			Crate->Destroy();
		}
	}
	const TArray<AHealthPack*>& HealthPacks = CachedGS->GetActiveHealthPacks();
	for (AHealthPack* HealthPack : HealthPacks)
	{
		if (HealthPack && !HealthPack->IsPendingKillPending())
		{
			HealthPack->Destroy();
		}
	}

	CachedGS->ClearActiveItems();
	CachedGS->SetMatchState(EMyMatchState::BUY_PHASE);
	CachedGS->SetHeroPhase(false);

	int BuyTime = 10; // seconds

	float Now = 0;
	if (const UWorld* World = GetWorld())
	{
		if (const AGameStateBase* GS = World->GetGameState())
		{
			Now = GS->GetServerWorldTimeSeconds();
		}
	}
	int TimeBuyEnd = Now + BuyTime;
	CachedGS->SetRoundEndTime(TimeBuyEnd);

	RestartAllPlayers();

	// reassign team id
	for (APlayerSlot* Slot : CachedGS->Slots)
	{
		Slot->SetTeamId(ETeamId::Soldier);
	}

	BotManager->OnStartRoundZombieMode();
	GetWorldTimerManager().SetTimer(
		BuyingTimerHandle,
		this,
		&AZombieMode::EnterFightState,
		BuyTime,
		false
	);
}

void AZombieMode::EnterFightState()
{
	CachedGS->SetMatchState(EMyMatchState::ROUND_IN_PROGRESS);

	float Now = 0;
	if (const UWorld* World = GetWorld())
	{
		if (const AGameStateBase* GS = World->GetGameState())
		{
			Now = GS->GetServerWorldTimeSeconds();
		}
	}
	int32 RoundProgressTimeEnd = Now + RoundProgressTime;
	CachedGS->SetRoundEndTime(RoundProgressTimeEnd);

	RandomZombie();

	GetWorld()->GetTimerManager().SetTimer(
		FightStateTimerHandle,
		this,
		&AZombieMode::OnRoundTimeExpired,
		RoundProgressTime,
		false
	);
	GetWorldTimerManager().SetTimer(
		AirdropCheckTimer,
		this,
		&AZombieMode::CheckAndSpawnAirdropCrate,
		20.0f,
		true
	);
	GetWorldTimerManager().SetTimer(
		HealPackCheckTimer,
		this,
		&AZombieMode::CheckAndSpawnHealPack,
		5.0f,
		true
	);
}

void AZombieMode::RandomZombie()
{
	// SERVER ONLY
	if (!HasAuthority())
	{
		return;
	}
	const int32 TotalPlayers = CachedGS->Slots.Num();
	
	int NumZombies = 0;
	if (TotalPlayers <= 3)
	{
		NumZombies = 1;
	}
	else if (TotalPlayers <= 6)
	{
		NumZombies = 2;
	}
	else {
		NumZombies = 3;
	}

	for (int32 i = 0; i < NumZombies; ++i)
	{
		ABaseCharacter* Pawn = ChooseZombie();
		if (!Pawn) continue;

		BecomeZombie(Pawn);
	}
}

// This function chooses a random player, but skips players who were zombies before
// Help prevent the same players from being zombies repeatedly
ABaseCharacter* AZombieMode::ChooseZombie() const {
	TArray<APlayerState*> PlayerStates = CachedGS->PlayerArray;
	TArray<AMyPlayerState*> Eligible;
	Eligible.Reserve(PlayerStates.Num());

	// Build eligible list
	for (APlayerState* PS : PlayerStates)
	{
		AMyPlayerState* MyPS = Cast<AMyPlayerState>(PS);
		if (!MyPS) continue;
		if (MyPS->WasChosenAsZombie()) continue; // skip players who were zombies before
		Eligible.Add(MyPS);
	}
	if (Eligible.Num() == 0)
	{
		// need to reset the "was zombie before" flags
		for (APlayerState* PS : PlayerStates)
		{
			AMyPlayerState* MyPS = Cast<AMyPlayerState>(PS);
			if (!MyPS) continue;
			MyPS->SetChosenAsZombie(false);
			Eligible.Add(MyPS);
		}
	}
	if (Eligible.Num() == 0)
	{
		return nullptr;
	}

	const int32 Index = FMath::RandRange(0, Eligible.Num() - 1);
	AMyPlayerState* ZombiePS = Eligible[Index];
	ZombiePS->SetChosenAsZombie(true);
	Eligible.RemoveAtSwap(Index);
	
	return Cast<ABaseCharacter>(ZombiePS->GetPawn());
}

void AZombieMode::BecomeZombie(ABaseCharacter* Character) {
	// update team Id because now he is zombie
	AMyPlayerState* PS = Cast<AMyPlayerState>(Character->GetPlayerState());
	if (!PS) {
		return;
	}
	PS->SetTeamId(ETeamId::Zombie);

	URoleComponent* RoleComp = Character->GetRoleComponent();
	if (RoleComp)
	{
		RoleComp->SetRoleAuthoritative(ECharacterRole::Zombie);
	}

	if (Character->IsBotControlled())
	{
		ABotAIController* BotCtrl = Cast<ABotAIController>(Character->GetController());
		if (BotCtrl) {
			BotManager->NotifyCharacterRole(BotCtrl, ECharacterRole::Zombie);
		}
	}
}

void AZombieMode::BecomeHero(ABaseCharacter* Character) {
	if (!Character) {
		return;
	}

	// check if is in hero phase
	if (!CachedGS->IsHeroPhase()) {
		return;
	}
	
	URoleComponent* RoleComp = Character->GetRoleComponent();
	if (RoleComp)
	{
		if (RoleComp->GetRole() != ECharacterRole::Human) {
			return; 
		}
		RoleComp->SetRoleAuthoritative(ECharacterRole::Hero);
	}
	
	if (Character->IsBotControlled())
	{
		ABotAIController* BotCtrl = Cast<ABotAIController>(Character->GetController());
		BotManager->NotifyCharacterRole(BotCtrl, ECharacterRole::Hero);
	}
}

void AZombieMode::EndRound(ETeamId WinningTeam)
{
	if (CachedGS->GetMatchState() == EMyMatchState::ROUND_ENDED)
	{
		return; // already ended
	}
	Super::EndRound(WinningTeam);

	CachedGS->SetMatchState(EMyMatchState::ROUND_ENDED);
	CachedGS->AddScoreTeam(WinningTeam, 1);
	CachedGS->Multicast_RoundResult(WinningTeam);
	GetWorld()->GetTimerManager().ClearTimer(FightStateTimerHandle);

	if (CachedGS->GetCurrentRound() >= FGameConstants::MAX_ROUND_ZOMBIE_MODE)
	{
		EndGame(WinningTeam);
		return;
	}

	// start new round after some delay
	GetWorldTimerManager().ClearTimer(StartRoundTimerHandle);
	GetWorldTimerManager().SetTimer(StartRoundTimerHandle, this, &AZombieMode::StartRound, DelayBeforeNewRound, false);
}

void AZombieMode::HandleCharacterKilled(const FCharacterKilledEvent& Event)
{
	Super::HandleCharacterKilled(Event);
	if (!Event.Victim)
	{
		return;
	}
	ABaseCharacter* VictimPawn = Event.Victim;
	const ECharacterRole VictimRole = VictimPawn->GetCharacterRole();

	switch (VictimRole)
	{
	case ECharacterRole::Human:
		HandleHumanKilled(VictimPawn);
		break;

	case ECharacterRole::Zombie:
		HandleZombieKilled(VictimPawn, Event.DamageCauser);
		break;

	case ECharacterRole::Hero:
		HandleHeroKilled(VictimPawn);
		break;

	default:
		break;
	}

}

void AZombieMode::HandleHumanKilled(ABaseCharacter* VictimPawn)
{
	BecomeZombie(VictimPawn);

	if (CachedGS->IsHeroPhase()) {
		CachedGS->SetRemainingHeroCount(CachedGS->GetRemainingHeroCount() - 1);
	}

	int TotalPlayers = CachedGS->Slots.Num();
	int AliveSoldiers = 0;
	for (APlayerSlot* Slot : CachedGS->Slots)
	{
		APawn* Pawn = Slot->GetPawn();
		if (!IsValid(Pawn)) continue;
		
		if (Slot->GetTeamId() == ETeamId::Soldier)
		{
			ABaseCharacter* SoldierChar = Cast<ABaseCharacter>(Pawn);
			if (!SoldierChar || SoldierChar->IsDead()) {
				continue;
			}
			AliveSoldiers++;
		}
	}
	if (AliveSoldiers == 0)
	{
		EndRound(ETeamId::Zombie);
	}
	else {
		if (!CachedGS->IsHeroPhase()) {
			// check condition to change to hero phase
			UE_LOG(LogTemp, Warning, TEXT("AZombieMode::HandleHumanKilled: TotalPlayers=%d, AliveSoldiers=%d"), TotalPlayers, AliveSoldiers);
			if (ShouldEnterHeroPhase(TotalPlayers, AliveSoldiers)) {
				// temporary hard code condition, will refactor later

				CachedGS->SetRemainingHeroCount(AliveSoldiers);
				CachedGS->SetRemainingZombieCount(TotalPlayers - AliveSoldiers);
				CachedGS->SetHeroPhase(true);

				// add time
				const int HeroProgressTime = 60; // add 1 minute
				float Now = 0;
				if (const UWorld* World = GetWorld())
				{
					if (const AGameStateBase* GS = World->GetGameState())
					{
						Now = GS->GetServerWorldTimeSeconds();
					}
				}
				int TimeEnd = Now + HeroProgressTime;
				CachedGS->SetRoundEndTime(TimeEnd);

				// reset timer
				GetWorld()->GetTimerManager().SetTimer(
					FightStateTimerHandle,
					this,
					&AZombieMode::OnRoundTimeExpired,
					HeroProgressTime,
					false
				);
			}
		}
	}
}

bool AZombieMode::ShouldEnterHeroPhase(int32 TotalPlayers, int32 AliveSoldiers) const {
	const bool bLargeLobbyEdgeCase =
		(TotalPlayers >= 8) && (AliveSoldiers >= 1) && (AliveSoldiers <= 2);

	const bool bSmallLobbyLastHero =
		(TotalPlayers >= 3) && (AliveSoldiers == 1);

	return bLargeLobbyEdgeCase || bSmallLobbyLastHero;
}

void AZombieMode::HandleZombieKilled(
	ABaseCharacter* VictimPawn,
	const UItemConfig* DamageCauser
)
{
	const bool bPermanentDead =
		DamageCauser &&
		DamageCauser->Id == EItemId::MELEE_SWORD_HERO; // special weapon that kills zombie permanently

	VictimPawn->ApplyRealDeath(false, bPermanentDead);

	if (bPermanentDead)
	{
		HandlePermanentZombieDeath(VictimPawn);
	}
	else
	{
		ScheduleZombieRevive(VictimPawn);
	}
}

void AZombieMode::HandleHeroKilled(ABaseCharacter* VictimPawn)
{
	VictimPawn->ApplyRealDeath(false, true); // hero always dies permanently
	StartSpectating(VictimPawn);

	// check if all soldiers are dead -> end game
	int RemainingHeroCount = CachedGS->GetRemainingHeroCount();
	RemainingHeroCount = FMath::Max(0, RemainingHeroCount - 1);
	CachedGS->SetRemainingHeroCount(RemainingHeroCount);
	
	if (RemainingHeroCount == 0)
	{
		EndRound(ETeamId::Zombie);
	}
}

void AZombieMode::HandlePermanentZombieDeath(ABaseCharacter* VictimPawn)
{
	if (!VictimPawn) return;

	StartSpectating(VictimPawn);

	// check if all zombie are dead -> end game with soldier win
	int ZombieRemainingCount = CachedGS->GetRemainingZombieCount();
	ZombieRemainingCount = FMath::Max(0, ZombieRemainingCount - 1);
	CachedGS->SetRemainingZombieCount(ZombieRemainingCount);

	bool bAllZombieDead = true;
	for (APlayerSlot* Slot : CachedGS->Slots)
	{
		if (Slot->GetTeamId() != ETeamId::Zombie)
		{
			continue;
		}
		APawn* Pawn = Slot->GetPawn();
		ABaseCharacter* MyChar = Cast<ABaseCharacter>(Pawn);
		if (!IsValid(MyChar)) continue;
		if (!MyChar->IsPermanentDead())
		{
			bAllZombieDead = false;
			break;
		}
	}
	if (bAllZombieDead)
	{
		EndRound(ETeamId::Soldier);
	}
}

void AZombieMode::ScheduleZombieRevive(ABaseCharacter* VictimPawn)
{
	if (!VictimPawn) return;

	constexpr float RespawnDelay = 2.f;
	TWeakObjectPtr<ABaseCharacter> PawnWeak = VictimPawn;

	FTimerHandle Handle;
	GetWorldTimerManager().SetTimer(
		Handle,
		[this, PawnWeak]()
		{
			if (!PawnWeak.IsValid()) return;

			ReviveZombie(PawnWeak.Get());
		},
		RespawnDelay,
		false
	);
}

void AZombieMode::ReviveZombie(ABaseCharacter* ZombieCharacter)
{
	if (!ZombieCharacter) return;
	ZombieCharacter->Revive();
	// teleport to new location
	AController* VictimCtrl = ZombieCharacter->GetController();
	if (VictimCtrl)
	{
		AActorManager* AM = AActorManager::Get(GetWorld());
		const FVector RandomLoc = AM->RandomLocationOnMap();
		const FRotator RandomRot = FRotator(0.f, FMath::FRandRange(0.f, 360.f), 0.f);

		ZombieCharacter->TeleportTo(RandomLoc, RandomRot, false, true);
		ZombieCharacter->ForceNetUpdate();
	}
}

void AZombieMode::EndGame(ETeamId WinningTeam)
{
	Super::EndGame(WinningTeam);
}

void AZombieMode::OnRoundTimeExpired()
{
	EndRound(ETeamId::Soldier);
}

void AZombieMode::StartSpectating(ABaseCharacter* VictimPawn) {
	if (!VictimPawn) return;
	AMyPlayerState* PS = VictimPawn->GetPlayerState<AMyPlayerState>();
	if (PS) {
		PS->SetIsSpectator(true);
	}
}

void AZombieMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	if (!NewPlayer) return;

	AActorManager* AM = AActorManager::Get(GetWorld());

	const FVector RandomLoc = AM->RandomLocationOnMap();
	const FRotator RandomRot = FRotator(0.f, FMath::FRandRange(0.f, 360.f), 0.f);

	FTransform SpawnTM(RandomRot, RandomLoc);
	// Spawns DefaultPawnClass at this transform and possesses it
	RestartPlayerAtTransform(NewPlayer, SpawnTM);
}

void AZombieMode::SpawnAirdropCrate() {
	AActorManager* AM = AActorManager::Get(GetWorld());
	FVector RandomLoc = AM->RandomLocationOnMap();
	RandomLoc.Z += 1500.f; // spawn above ground
	const FRotator RandomRot = FRotator::ZeroRotator;
	FTransform SpawnTM(RandomRot, RandomLoc);
	AAirdropCrate* Crate = GetWorld()->SpawnActor<AAirdropCrate>(AAirdropCrate::StaticClass(), SpawnTM);
	CachedGS->OnSpawnedAirdropCrate(Crate);
}

void AZombieMode::CheckAndSpawnAirdropCrate() {
	if (CachedGS->GetMatchState() != EMyMatchState::ROUND_IN_PROGRESS) {
		return; // only spawn during round in progress
	}
	const int CurrentAirdropNum = CachedGS->GetActiveAirdropCrates().Num();
	const int MaxAirdropNum = 2;
	if (CurrentAirdropNum < MaxAirdropNum) {
		SpawnAirdropCrate();
	}
}

void AZombieMode::CheckAndSpawnHealPack() {
	if (CachedGS->GetMatchState() != EMyMatchState::ROUND_IN_PROGRESS) {
		return; // only spawn during round in progress
	}
	const int CurrentHealthPackNum = CachedGS->GetActiveHealthPacks().Num();
	const int MaxHealthPackNum = 2;
	if (CurrentHealthPackNum < MaxHealthPackNum) {
		SpawnHealPack();
	}
}

void AZombieMode::SpawnHealPack() {
	AActorManager* AM = AActorManager::Get(GetWorld());
	FVector RandomLoc = AM->RandomLocationOnMap();
	RandomLoc.Z += 80.f; // spawn above ground
	const FRotator RandomRot = FRotator::ZeroRotator;
	FTransform SpawnTM(RandomRot, RandomLoc);
	auto HealPack = GetWorld()->SpawnActor<AHealthPack>(AHealthPack::StaticClass(), SpawnTM);
	CachedGS->OnSpawnedHealthPack(HealPack);
}