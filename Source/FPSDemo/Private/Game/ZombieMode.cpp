// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/ZombieMode.h"
#include "Game/ShooterGameState.h"
#include "Characters/BaseCharacter.h"
#include "Components/RoleComponent.h"
#include "Game/ActorManager.h"
#include <GameFramework/PlayerStart.h>

void AZombieMode::StartPlay()
{
	UE_LOG(LogTemp, Warning, TEXT("SpikeMode Game Started!"));
	Super::StartPlay();

	AShooterGameState* GS = GetGameState<AShooterGameState>();
	GS->SetMatchMode(EMatchMode::Zombie);
	BotManager->SetMatchMode(EMatchMode::Zombie);

	SpawnBot("B");
	SpawnBot("B");
	SpawnBot("B");
	SpawnBot("B");
}

void AZombieMode::StartRound()
{
	UE_LOG(LogTemp, Warning, TEXT("AZombieMode: Starting Round..."));
	Super::StartRound();

	AShooterGameState* GS = GetGameState<AShooterGameState>();
	if (GS) {
		GS->SetMatchState(EMyMatchState::BUY_PHASE);
	}

	int BuyTime = 15; // seconds
	int TimeBuyEnd = GetWorld()->GetTimeSeconds() + BuyTime;
	GS->SetRoundEndTime(TimeBuyEnd);
	ResetPlayers();

	BotManager->OnStartRoundZombieMode();

	GetWorldTimerManager().SetTimer(
		RoleAssignTimerHandle,
		this,
		&AZombieMode::AssignZombieRoles,
		15.0f,
		false
	);
}

void AZombieMode::AssignZombieRoles()
{
	AShooterGameState* GS = GetGameState<AShooterGameState>();
	if (!GS) return;
	GS->SetMatchState(EMyMatchState::ROUND_IN_PROGRESS);
	int RoundProgressTime = 180; // seconds - 3 minutes
	int RoundProgressTimeEnd = GetWorld()->GetTimeSeconds() + RoundProgressTime;
	GS->SetRoundEndTime(RoundProgressTimeEnd);

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

	// Assign roles: only 1 zombie
	for (AMyPlayerState* MyPS : Eligible)
	{
		ABaseCharacter* MyChar = Cast<ABaseCharacter>(MyPS->GetPawn());
		if (!MyChar) continue;

		URoleComponent* RoleComp = MyChar->GetRoleComponent();
		if (!RoleComp) continue;

		const bool bMakeZombie = (MyPS == ZombiePS);
		if (bMakeZombie) {
			RoleComp->SetRoleAuthoritative(ECharacterRole::Zombie);

			if (ABotAIController* BotCtrl = Cast<ABotAIController>(MyPS->GetOwner()))
			{
				BotManager->NotifyCharacterRole(BotCtrl, ECharacterRole::Zombie);
			}
		}
	}

}

void AZombieMode::EndRound(FName WinningTeam)
{
	Super::EndRound(WinningTeam);
	UE_LOG(LogTemp, Warning, TEXT("Round Ended! Team %s wins!"), *WinningTeam.ToString());
	AShooterGameState* GS = GetGameState<AShooterGameState>();
	if (GS)
	{
		GS->SetMatchState(EMyMatchState::ROUND_ENDED);
		GS->Multicast_RoundResult(WinningTeam);
	}
}

void AZombieMode::NotifyPlayerKilled(class AController* Killer, class ABaseCharacter* VictimPawn, const UItemConfig* DamageCauser, bool bWasHeadShot)
{
	Super::NotifyPlayerKilled(Killer, VictimPawn, DamageCauser, bWasHeadShot);

	AController* Victim = VictimPawn->GetController();

	// spawn victim as zombie
	ECharacterRole CurrentRole = VictimPawn->GetCharacterRole();
	if (CurrentRole == ECharacterRole::Hero)
	{
		// end game
		EndRound("B"); // zombie team wins
		return; // already a zombie
	}

	Victim->UnPossess(); // no need old pawn
	VictimPawn->SetLifeSpan(5.0f); // cleanup old pawn
	bool bIsCurrentZombie = VictimPawn->GetCharacterRole() == ECharacterRole::Zombie;

	UE_LOG(LogTemp, Warning, TEXT("Respawning player as zombie..."));
	RestartPlayer(Victim);

	ABaseCharacter* NewChar = Cast<ABaseCharacter>(Victim->GetPawn());
	if (NewChar)
	{
		URoleComponent* RoleComp = NewChar->FindComponentByClass<URoleComponent>();
		if (RoleComp)
		{
			RoleComp->SetRoleAuthoritative(ECharacterRole::Zombie);

			if (ABotAIController* BotCtrl = Cast<ABotAIController>(Victim))
			{
				BotManager->NotifyCharacterRole(BotCtrl, ECharacterRole::Zombie);
			}
		}
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