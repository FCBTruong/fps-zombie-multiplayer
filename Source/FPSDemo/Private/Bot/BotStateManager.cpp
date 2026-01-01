// Fill out your copyright notice in the Description page of Project Settings.


#include "Bot/BotStateManager.h"
#include "Bot/BotRole.h"
#include "Controllers/BotAIController.h"
#include "Controllers/MyPlayerState.h"
#include "Game/ActorManager.h"
#include "Engine/TriggerBox.h"
#include "Characters/BaseCharacter.h"

BotStateManager::BotStateManager()
{
}

BotStateManager::~BotStateManager()
{
}

void BotStateManager::AddBot(ABotAIController* NewBot)
{
	ManagedBots.Add(NewBot);
}

void BotStateManager::OnSpikePlanted(FName AttackerTeamId, AActor* SpikeActor)
{
	for (ABotAIController* Bot : ManagedBots)
	{
		// Bot->ReactToSpikePlanted();
	}

	const EBotRole AttackerRoles[] =
	{
		EBotRole::Scout
	};
	const EBotRole DefenderRoles[] =
	{
		EBotRole::Scout
	};

	TArray<ABotAIController*> Defenders;
	for (ABotAIController* Bot : ManagedBots)
	{
		AMyPlayerState* PS = Bot->GetPlayerState<AMyPlayerState>();
		if (PS->IsAlive() == false) continue;

		bool IsAttacker = (PS->GetTeamID() == AttackerTeamId);
		if (IsAttacker) {
			const int32 Index = FMath::RandRange(0, UE_ARRAY_COUNT(AttackerRoles) - 1);
			const EBotRole ChosenRole = AttackerRoles[Index];
			Bot->SetSpikeRole(ChosenRole);
		}
		else {
			const int32 Index = FMath::RandRange(0, UE_ARRAY_COUNT(DefenderRoles) - 1);
			const EBotRole ChosenRole = DefenderRoles[Index];
			Bot->SetSpikeRole(ChosenRole);
			Defenders.Add(Bot);
		}
		Bot->SetSpikeActor(SpikeActor);
	}
	// assign one defender as spike defuser
	if (Defenders.Num() > 0)
	{
		int32 RandomIndex = FMath::RandRange(0, Defenders.Num() - 1);
		ABotAIController* SpikeDefuserBot = Defenders[RandomIndex];
		SpikeDefuserBot->SetSpikeRole(EBotRole::D_Defuser);
	}
}

void BotStateManager::OnSpikeDefused()
{
	for (ABotAIController* Bot : ManagedBots)
	{
		// Bot->ReactToSpikeDefused();
	}
}

void BotStateManager::OnSpikePickedUp(ABaseCharacter* Player)
{
	UE_LOG(LogTemp, Warning, TEXT("BotStateManager: OnSpikePickedUp called by player"));
	if (!Player) return;
	ABotAIController* PlayerBot = Cast<ABotAIController>(Player->GetController());
	// find the bot that picked up the spike
	for (ABotAIController* Bot : ManagedBots)
	{
		if (Bot != PlayerBot) {
			EBotRole CurrentRole = Bot->GetSpikeRole();
			if (CurrentRole == EBotRole::A_FindSpike) {
				Bot->SetSpikeRole(EBotRole::Scout);
			}
		}
	}
	if (!PlayerBot) return;
	PlayerBot->SetSpikeRole(EBotRole::A_Carrier);
}

void BotStateManager::OnSpikeDropped()
{
	for (ABotAIController* Bot : ManagedBots)
	{
		// Bot->ReactToSpikeDropped();
	}
}

void BotStateManager::OnSpikeCarrierKilled()
{
	for (ABotAIController* Bot : ManagedBots)
	{
		// Bot->ReactToSpikeCarrierKilled();
	}
}

void BotStateManager::RemoveBot(ABotAIController* BotToRemove)
{
	ManagedBots.Remove(BotToRemove);
}

void BotStateManager::OnStartRound(AActorManager* ActorMgr, FName AttackerTeamId, AActor* SpikeActor)
{
	FName BombSite = FMath::RandBool() ? FName(TEXT("A")) : FName(TEXT("B"));
	FVector PlantLocation =
		(BombSite == FName(TEXT("A")))
		? ActorMgr->GetAreaBombA()->GetActorLocation()
		: ActorMgr->GetAreaBombB()->GetActorLocation();

	TArray<ABotAIController*> Attackers;
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
	for (ABotAIController* Bot : ManagedBots)
	{
		Bot->ResetAIState();
		AMyPlayerState* PS = Bot->GetPlayerState<AMyPlayerState>();
		bool IsAttacker = (PS->GetTeamID() == AttackerTeamId);

		Bot->SetIsAttacker(IsAttacker);

		Bot->SetSpikeActor(SpikeActor);
		Bot->SetPlantLocation(PlantLocation);

		if (IsAttacker)
		{
			const int32 Index = FMath::RandRange(0, UE_ARRAY_COUNT(AttackerRoles) - 1);
			const EBotRole ChosenRole = AttackerRoles[Index];
			Bot->SetSpikeRole(ChosenRole);
			Attackers.Add(Bot);
		}
		else {
			const int32 Index = FMath::RandRange(0, UE_ARRAY_COUNT(DefenderRoles) - 1);
			const EBotRole ChosenRole = DefenderRoles[Index];
			Bot->SetSpikeRole(ChosenRole);
		}
	}

	// auto assign spike carrier for one of the attackers
	if (Attackers.Num() > 0)
	{
		int32 RandomIndex = FMath::RandRange(0, Attackers.Num() - 1);
		ABotAIController* SpikeCarrierBot = Attackers[RandomIndex];
		SpikeCarrierBot->SetSpikeRole(EBotRole::A_Carrier);
	}
}

void BotStateManager::NotifyCharacterRole(ABotAIController* Bot, ECharacterRole NewRole) {
	if (!Bot) return;
	Bot->SetCharacterRole(NewRole);
}