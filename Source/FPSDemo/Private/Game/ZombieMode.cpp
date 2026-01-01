// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/ZombieMode.h"
#include "Game/ShooterGameState.h"
#include "Characters/BaseCharacter.h"
#include "Components/RoleComponent.h"

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