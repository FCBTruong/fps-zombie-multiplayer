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

	FName AttackerTeam = (FMath::RandBool()) ? FName("A") : FName("B");
	AttackerTeam = "A"; // for testing
	GS->SetAttackerTeam(AttackerTeam);
	SpawnBot("B");
	SpawnBot("B");
	SpawnBot("B");
	SpawnBot("B");
}

void AZombieMode::StartRound()
{
	UE_LOG(LogTemp, Warning, TEXT("AZombieMode: Starting Round..."));
	AShooterGameState* GS = GetGameState<AShooterGameState>();
	if (GS) {
		GS->SetMatchState(EMyMatchState::ROUND_IN_PROGRESS);
	}
	ResetPlayers();

	GetWorldTimerManager().SetTimer(
		RoleAssignTimerHandle,
		this,
		&AZombieMode::AssignZombieRoles,
		5.0f,
		false
	);
}

void AZombieMode::AssignZombieRoles()
{
	UE_LOG(LogTemp, Warning, TEXT("Assigning Zombie Roles to Bots..."));
	if (!BotManager) return;

	const bool bMakeZombie = true;

	for (auto& Bot : BotManager->GetManagedBots())
	{
		if (!Bot) continue;

		ABaseCharacter* BotChar = Cast<ABaseCharacter>(Bot->GetPawn());
		if (!BotChar)
		{
			UE_LOG(LogTemp, Warning, TEXT("Bot pawn is null in AssignZombieRoles"));
			continue;
		}

		URoleComponent* RoleComp = BotChar->FindComponentByClass<URoleComponent>();
		if (!RoleComp) continue;

		RoleComp->SetRoleAuthoritative(
			bMakeZombie ? ECharacterRole::Zombie : ECharacterRole::Human
		);

		BotManager->NotifyCharacterRole(Bot, bMakeZombie ? ECharacterRole::Zombie : ECharacterRole::Human);
	}
}

void AZombieMode::EndRound(FName WinningTeam)
{
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
