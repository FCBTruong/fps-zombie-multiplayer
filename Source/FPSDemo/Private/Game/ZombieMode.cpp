// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/ZombieMode.h"
#include "Game/ShooterGameState.h"
#include "Characters/BaseCharacter.h"
#include "Components/RoleComponent.h"
#include "Game/ActorManager.h"
#include "GameFramework/PlayerStart.h"
#include "Items/ItemConfig.h"
#include "Controllers/MyPlayerController.h"
#include "Items/AirdropCrate.h"
#include "Components/InventoryComponent.h"
#include "Game/PlayerSlot.h"

void AZombieMode::StartPlay()
{
	UE_LOG(LogTemp, Warning, TEXT("SpikeMode Game Started!"));
	Super::StartPlay();

}

void AZombieMode::StartRound()
{
	UE_LOG(LogTemp, Warning, TEXT("AZombieMode: Starting Round..."));
	Super::StartRound();


	// clean airdrop crates and clear timer
	GetWorldTimerManager().ClearTimer(AirdropCheckTimer);
	auto Airdopes = CachedGS->GetActiveAirdropCrates();
	for (AAirdropCrate* Crate : Airdopes)
	{
		if (Crate && !Crate->IsPendingKillPending())
		{
			Crate->Destroy();
		}
	}
	CachedGS->ClearAirdropCrates();
	
	CachedGS->SetMatchState(EMyMatchState::BUY_PHASE);
	CachedGS->SetHeroPhase(false);

	int BuyTime = 10; // seconds
	int TimeBuyEnd = GetWorld()->GetTimeSeconds() + BuyTime;
	CachedGS->SetRoundEndTime(TimeBuyEnd);
	ResetPlayers();

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
	UE_LOG(LogTemp, Warning, TEXT("AZombieMode: Entering Fight State..."));
	AShooterGameState* GS = GetGameState<AShooterGameState>();
	if (!GS) return;
	GS->SetMatchState(EMyMatchState::ROUND_IN_PROGRESS);
	int RoundProgressTime = 180; // seconds - 3 minutes
	int RoundProgressTimeEnd = GetWorld()->GetTimeSeconds() + RoundProgressTime;
	GS->SetRoundEndTime(RoundProgressTimeEnd);

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
		AController* Ctrl = ChooseZombieController();
		if (!Ctrl) continue;

		BecomeZombie(Ctrl);
	}
}

// This function chooses a random player, but skips players who were zombies before
// Help prevent the same players from being zombies repeatedly
AController* AZombieMode::ChooseZombieController() const {
	AShooterGameState* GS = GetGameState<AShooterGameState>();
	if (!GS) return nullptr;

	TArray<APlayerState*> PlayerStates = GS->PlayerArray;

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
		UE_LOG(LogTemp, Warning, TEXT("ChooseZombieController: No eligible players found"));
		return nullptr;
	}

	const int32 Index = FMath::RandRange(0, Eligible.Num() - 1);
	AMyPlayerState* ZombiePS = Eligible[Index];
	ZombiePS->SetChosenAsZombie(true);
	Eligible.RemoveAtSwap(Index);
	return Cast<AController>(ZombiePS->GetOwner());
}

void AZombieMode::BecomeZombie(AController* Controller) {
	UE_LOG(LogTemp, Warning, TEXT("AZombieMode::BecomeZombie called"));
	// update team Id because now he is zombie
	AMyPlayerState* PS = Controller->GetPlayerState<AMyPlayerState>();
	if (!PS) {
		UE_LOG(LogTemp, Warning, TEXT("PlayerState is null in BecomeZombie"));
		return;
	}

	AShooterGameState* GS = GetGameState<AShooterGameState>();
	if (GS) {

	}
	PS->SetTeamId(ETeamId::Zombie);

	if (ABaseCharacter* Character = Cast<ABaseCharacter>(Controller->GetPawn()))
	{		
		URoleComponent* RoleComp = Character->GetRoleComponent();
		
		if (RoleComp)
		{
			RoleComp->SetRoleAuthoritative(ECharacterRole::Zombie);
		}
	}
	if (ABotAIController* BotCtrl = Cast<ABotAIController>(Controller))
	{
		BotManager->NotifyCharacterRole(BotCtrl, ECharacterRole::Zombie);
	}
}

void AZombieMode::BecomeHero(AController* Controller) {
	UE_LOG(LogTemp, Warning, TEXT("AZombieMode::BecomeHero called"));
	// check if is in hero phase
	AShooterGameState* GS = GetGameState<AShooterGameState>();
	if (!GS || !GS->IsHeroPhase()) {
		UE_LOG(LogTemp, Warning, TEXT("AZombieMode::BecomeHero: Not in hero phase"));
		return;
	}

	if (ABaseCharacter* Character = Cast<ABaseCharacter>(Controller->GetPawn()))
	{
		URoleComponent* RoleComp = Character->GetRoleComponent();
		if (RoleComp)
		{
			if (RoleComp->GetRole() != ECharacterRole::Human) {
				UE_LOG(LogTemp, Warning, TEXT("AZombieMode::BecomeHero: Not allowed"));
				return; 
			}
			RoleComp->SetRoleAuthoritative(ECharacterRole::Hero);
		}
	}
	if (ABotAIController* BotCtrl = Cast<ABotAIController>(Controller))
	{
		BotManager->NotifyCharacterRole(BotCtrl, ECharacterRole::Hero);
	}
}

void AZombieMode::EndRound(ETeamId WinningTeam)
{
	Super::EndRound(WinningTeam);

	AShooterGameState* GS = GetGameState<AShooterGameState>();
	if (!GS)
	{
		return;
	}
	if (GS->GetMatchState() == EMyMatchState::ROUND_ENDED)
	{
		return; // already ended
	}
	GS->SetMatchState(EMyMatchState::ROUND_ENDED);
	GS->AddScoreTeam(WinningTeam, 1);
	GS->Multicast_RoundResult(WinningTeam);
	GetWorld()->GetTimerManager().ClearTimer(FightStateTimerHandle);

	// start new round after some delay
	constexpr float DelayBeforeNewRound = 3.f;
	GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
		{
			GetWorldTimerManager().SetTimer(
				StartRoundTimerHandle,
				[this]()
				{
					StartRound();
				},
				DelayBeforeNewRound,
				false
			);
		});
}

void AZombieMode::HandleCharacterKilled(AController* Killer, const TArray<TWeakObjectPtr<AController>>& Assists, ABaseCharacter* VictimPawn, const UItemConfig* DamageCauser, bool bWasHeadShot)
{
	Super::HandleCharacterKilled(Killer, Assists, VictimPawn, DamageCauser, bWasHeadShot);
	UE_LOG(LogTemp, Warning, TEXT("AZombieMode::OnCharacterKilled called"));

	if (!VictimPawn)
	{
		return;
	}

	const ECharacterRole VictimRole = VictimPawn->GetCharacterRole();

	switch (VictimRole)
	{
	case ECharacterRole::Human:
		HandleHumanKilled(VictimPawn);
		break;

	case ECharacterRole::Zombie:
		HandleZombieKilled(VictimPawn, DamageCauser);
		break;

	case ECharacterRole::Hero:
		HandleHeroKilled(VictimPawn);
		break;

	default:
		UE_LOG(LogTemp, Warning,
			TEXT("OnCharacterKilled: Unhandled role %d"),
			static_cast<int32>(VictimRole));
		break;
	}

}

void AZombieMode::HandleHumanKilled(ABaseCharacter* VictimPawn)
{
	if (AController* Ctrl = VictimPawn->GetController())
	{
		BecomeZombie(Ctrl);
	}

	// check if all is zombie -> end game with zombie win
	AShooterGameState* GS = GetGameState<AShooterGameState>();
	if (!GS) return;

	bool bAllZombie = true;
	int TotalPlayers = GS->Slots.Num();
	int AliveSoldiers = 0;
	for (APlayerSlot* Slot : GS->Slots)
	{
		APawn* Pawn = Slot->GetPawn();
		if (!IsValid(Pawn)) continue;
		
		if (Slot->GetTeamId() == ETeamId::Soldier)
		{
			ABaseCharacter* SoldierChar = Cast<ABaseCharacter>(Pawn);
			if (SoldierChar && SoldierChar->IsDead()) {
				continue;
			}
			AliveSoldiers++;
			bAllZombie = false;
		}
	}
	if (bAllZombie)
	{
		EndRound(ETeamId::Zombie);
	}
	else {
		if (!GS->IsHeroPhase()) {
			// check condition to change to hero phase
			UE_LOG(LogTemp, Warning, TEXT("AZombieMode::HandleHumanKilled: TotalPlayers=%d, AliveSoldiers=%d"), TotalPlayers, AliveSoldiers);
			if ((TotalPlayers >= 8 && AliveSoldiers == 2)
				or (TotalPlayers >= 3 && AliveSoldiers == 1)) {
				// temporary hard code condition, will refactor later

				GS->SetRemainingHeroCount(AliveSoldiers);
				GS->SetRemainingZombieCount(TotalPlayers - AliveSoldiers);
				GS->SetHeroPhase(true);

				// add time
				const int HeroProgressTime = 60; // add 1 minute
				int TimeEnd = GetWorld()->GetTimeSeconds() + HeroProgressTime;
				GS->SetRoundEndTime(TimeEnd);

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

void AZombieMode::HandleZombieKilled(
	ABaseCharacter* VictimPawn,
	const UItemConfig* DamageCauser
)
{
	const bool bPermanentDead =
		DamageCauser &&
		DamageCauser->Id == EItemId::MELEE_SWORD_HERO; // special weapon that kills zombie permanently

	VictimPawn->ApplyRealDeath(false);

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
	VictimPawn->ApplyRealDeath(false);
	StartSpectating(VictimPawn);

	// check if all soldiers are dead -> end game
	AShooterGameState* GS = GetGameState<AShooterGameState>();
	if (!GS) return;
	int RemainingHeroCount = GS->GetRemainingHeroCount();
	RemainingHeroCount = FMath::Max(0, RemainingHeroCount - 1);
	GS->SetRemainingHeroCount(RemainingHeroCount);
	
	if (RemainingHeroCount == 0)
	{
		EndRound(ETeamId::Zombie);
	}
}

void AZombieMode::HandlePermanentZombieDeath(ABaseCharacter* VictimPawn)
{
	if (!VictimPawn) return;

	StartSpectating(VictimPawn);
	VictimPawn->SetPermanentDead(true);

	// check if all zombie are dead -> end game with soldier win
	AShooterGameState* GS = GetGameState<AShooterGameState>();
	if (!GS) return;
	int ZombieRemainingCount = GS->GetRemainingZombieCount();
	ZombieRemainingCount = FMath::Max(0, ZombieRemainingCount - 1);
	GS->SetRemainingZombieCount(ZombieRemainingCount);

	bool bAllZombieDead = true;
	for (APlayerSlot* Slot : GS->Slots)
	{
		if (Slot->GetTeamId() != ETeamId::Zombie)
		{
			continue;
		}
		APawn* Pawn = Slot->GetPawn();
		if (!IsValid(Pawn)) continue;
		ABaseCharacter* MyChar = Cast<ABaseCharacter>(Pawn);
		if (!MyChar) continue;
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

		FTransform SpawnTM(RandomRot, RandomLoc);

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
	AShooterGameState* GS = GetGameState<AShooterGameState>();	
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
	AShooterGameState* GS = GetGameState<AShooterGameState>();
	if (!GS) return;

	AActorManager* AM = AActorManager::Get(GetWorld());
	FVector RandomLoc = AM->RandomLocationOnMap();
	RandomLoc.Z += 1500.f; // spawn above ground
	const FRotator RandomRot = FRotator::ZeroRotator;
	FTransform SpawnTM(RandomRot, RandomLoc);
	auto Crate = GetWorld()->SpawnActor<AAirdropCrate>(AAirdropCrate::StaticClass(), SpawnTM);
	GS->OnSpawnedAirdropCrate(Crate);
	if (Crate) {
		Crate->OnAirdropClaimed.AddUObject(this, &AZombieMode::HandleAirdropClaimed);
	}
}

void AZombieMode::HandleAirdropClaimed(AAirdropCrate* AirdropCrate, ABaseCharacter* Character) {
	if (!AirdropCrate || !Character) return;
	
	AShooterGameState* GS = GetGameState<AShooterGameState>();
	if (!GS) return;

	EItemId GiftId = EItemId::NONE;

	int32 A = FMath::RandRange(1, 100);
	/*if (A > 80)
	{
		GiftId = static_cast<EItemId>(
			FMath::RandRange(
				static_cast<int32>(EItemId::RIFLE_M16A),
				static_cast<int32>(EItemId::RIFLE_QBZ)
			)
			);
	}*/
	GS->OnClaimedAirdropCrate(AirdropCrate, Character, GiftId);
	// add gifts to character
	// right now only bullets to main gun
	Character->GetInventoryComponent()->AddAmmoToMainGun(90); // add 90 bullets
	AirdropCrate->Destroy();
}

void AZombieMode::CheckAndSpawnAirdropCrate() {
	AShooterGameState* GS = GetGameState<AShooterGameState>();
	if (!GS) return;
	if (GS->GetMatchState() != EMyMatchState::ROUND_IN_PROGRESS) {
		return; // only spawn during round in progress
	}
	const int CurrentAirdropNum = GS->GetActiveAirdropCrates().Num();
	const int MaxAirdropNum = 2;
	if (CurrentAirdropNum < MaxAirdropNum) {
		SpawnAirdropCrate();
	}
}