// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/ZombieMode.h"
#include "Game/ShooterGameState.h"
#include "Characters/BaseCharacter.h"
#include "Components/RoleComponent.h"
#include "Game/ActorManager.h"
#include "GameFramework/PlayerStart.h"
#include "Items/ItemConfig.h"
#include "Controllers/MyPlayerController.h"

void AZombieMode::StartPlay()
{
	UE_LOG(LogTemp, Warning, TEXT("SpikeMode Game Started!"));
	Super::StartPlay();

	SpawnBot(ETeamId::None);
	SpawnBot(ETeamId::None);
	SpawnBot(ETeamId::None);
	SpawnBot(ETeamId::None);
}

void AZombieMode::StartRound()
{
	UE_LOG(LogTemp, Warning, TEXT("AZombieMode: Starting Round..."));
	Super::StartRound();

	AShooterGameState* GS = GetGameState<AShooterGameState>();
	if (GS) {
		GS->SetMatchState(EMyMatchState::BUY_PHASE);
	}

	int BuyTime = 3; // seconds
	int TimeBuyEnd = GetWorld()->GetTimeSeconds() + BuyTime;
	GS->SetRoundEndTime(TimeBuyEnd);
	ResetPlayers();

	// reassign team id
	for (APlayerState* PS : GS->PlayerArray)
	{
		AMyPlayerState* MyPS = Cast<AMyPlayerState>(PS);
		if (!MyPS) continue;
		MyPS->SetTeamId(ETeamId::Soldier);
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
}

void AZombieMode::RandomZombie()
{
	AShooterGameState* GS = GetGameState<AShooterGameState>();
	if (!GS) return;
	auto PlayerStates = GS->PlayerArray;

	// Build eligible list (valid PS + valid pawn + valid role comp)
	TArray<AMyPlayerState*> Eligible;
	Eligible.Reserve(PlayerStates.Num());

	for (APlayerState* PS : PlayerStates)
	{
		AMyPlayerState* MyPS = Cast<AMyPlayerState>(PS);
		if (!MyPS) continue;

		ABaseCharacter* MyChar = Cast<ABaseCharacter>(MyPS->GetPawn());
		if (!MyChar) continue;

		URoleComponent* RoleComp = MyChar->GetRoleComponent();
		if (!RoleComp) continue;

		Eligible.Add(MyPS);
	}

	if (Eligible.Num() == 0)
	{
		return;
	}

	const int32 ZombieIdx = FMath::RandRange(0, Eligible.Num() - 1);
	AMyPlayerState* ZombiePS = Eligible[ZombieIdx];
	if (!ZombiePS) return;

	AController* Ctrl = Cast<AController>(ZombiePS->GetOwner());
	if (!Ctrl) return;
	BecomeZombie(Ctrl);
}

void AZombieMode::BecomeZombie(AController* Controller) {
	UE_LOG(LogTemp, Warning, TEXT("AZombieMode::BecomeZombie called"));
	// update team Id because now he is zombie
	AMyPlayerState* PS = Controller->GetPlayerState<AMyPlayerState>();
	if (PS) {
		PS->SetTeamId(ETeamId::Zombie);
	}
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
	if (ABaseCharacter* Character = Cast<ABaseCharacter>(Controller->GetPawn()))
	{
		URoleComponent* RoleComp = Character->GetRoleComponent();
		if (RoleComp)
		{
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
	if (GS)
	{
		GS->SetMatchState(EMyMatchState::ROUND_ENDED);
		GS->Multicast_RoundResult(WinningTeam);
	}

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

void AZombieMode::OnCharacterKilled(class AController* Killer, class ABaseCharacter* VictimPawn, const UItemConfig* DamageCauser, bool bWasHeadShot)
{
	Super::OnCharacterKilled(Killer, VictimPawn, DamageCauser, bWasHeadShot);
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

AActor* AZombieMode::ChoosePlayerStart_Implementation(AController* Player)
{
	UE_LOG(LogTemp, Warning, TEXT("DEBUGXX: ChoosePlayerStart_Implementation..."));
	AMyPlayerState* PS = Player->GetPlayerState<AMyPlayerState>();

	if (!PS) {
		UE_LOG(LogTemp, Warning, TEXT("PlayerState is null in ChoosePlayerStart_Implementation"));
		return Super::ChoosePlayerStart_Implementation(Player); // fallback
	}

	AActorManager* AM = AActorManager::Get(GetWorld());

	APlayerStart* PlayerStart = AM->GetRandomZombieStart();
	if (PlayerStart) {
		return PlayerStart;
	}

	return Super::ChoosePlayerStart_Implementation(Player); // fallback
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
	auto PlayerStates = GS->PlayerArray;
	bool bAllZombie = true;
	for (APlayerState* PS : PlayerStates)
	{
		AMyPlayerState* MyPS = Cast<AMyPlayerState>(PS);
		if (!MyPS) continue;
		if (MyPS->GetTeamId() != ETeamId::Zombie)
		{
			bAllZombie = false;
			break;
		}
	}
	if (bAllZombie)
	{
		EndRound(ETeamId::Zombie);
	}
}

void AZombieMode::HandleZombieKilled(
	ABaseCharacter* VictimPawn,
	const UItemConfig* DamageCauser
)
{
	const bool bPermanentDead =
		DamageCauser &&
		DamageCauser->GetItemType() == EItemType::Melee;

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
	auto PlayerStates = GS->PlayerArray;
	bool bAllSoldierDead = true;
	for (APlayerState* PS : PlayerStates)
	{
		AMyPlayerState* MyPS = Cast<AMyPlayerState>(PS);
		if (!MyPS) continue;
		ABaseCharacter* MyChar = Cast<ABaseCharacter>(MyPS->GetPawn());
		if (!MyChar) continue;
		if (MyChar->IsDead()) continue;
		if (!MyChar->IsZombie())
		{
			bAllSoldierDead = false;
			break;
		}
	}
	if (bAllSoldierDead)
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
	auto PlayerStates = GS->PlayerArray;
	bool bAllZombieDead = true;
	for (APlayerState* PS : PlayerStates)
	{
		AMyPlayerState* MyPS = Cast<AMyPlayerState>(PS);
		if (!MyPS) continue;
		if (MyPS->GetTeamId() != ETeamId::Zombie)
		{
			continue;
		}
		ABaseCharacter* MyChar = Cast<ABaseCharacter>(MyPS->GetPawn());
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


APawn* AZombieMode::SpawnDefaultPawnAtTransform_Implementation(
	AController* NewPlayer,
	const FTransform& SpawnTransform
)
{
	FTransform NewTransform = SpawnTransform;

	FRotator Rot = NewTransform.GetRotation().Rotator();
	Rot.Yaw = FMath::RandRange(0.f, 360.f);
	NewTransform.SetRotation(Rot.Quaternion());

	return Super::SpawnDefaultPawnAtTransform_Implementation(NewPlayer, NewTransform);
}

void AZombieMode::ReviveZombie(ABaseCharacter* ZombieCharacter)
{
	if (!ZombieCharacter) return;

	// teleport to new location
	AController* VictimCtrl = ZombieCharacter->GetController();
	if (VictimCtrl)
	{
		AActor* StartSpot = ChoosePlayerStart_Implementation(VictimCtrl);
		if (StartSpot)
		{
			const FVector Loc = StartSpot->GetActorLocation();
			const FRotator Rot = StartSpot->GetActorRotation();

			ZombieCharacter->TeleportTo(Loc, Rot, false, true);
			ZombieCharacter->ForceNetUpdate();
		}
	}

	ZombieCharacter->Revive();
}

void AZombieMode::EndGame(ETeamId WinningTeam)
{

}

void AZombieMode::OnRoundTimeExpired()
{
	AShooterGameState* GS = GetGameState<AShooterGameState>();	
	EndRound(ETeamId::Soldier);
}

void AZombieMode::StartSpectating(ABaseCharacter* VictimPawn) {
	if (!VictimPawn) return;
	AMyPlayerController* PC =
		Cast<AMyPlayerController>(VictimPawn->GetController());
	if (!PC) return;

	// Move anyone watching this pawn
	MoveSpectatorsOffDeadPawn(VictimPawn);

	TWeakObjectPtr<AMyPlayerController> PCWeak = PC;

	GetWorldTimerManager().SetTimerForNextTick([PCWeak]()
		{
			if (!PCWeak.IsValid()) return;

			PCWeak->SetPlayerSpectate();
		});
}