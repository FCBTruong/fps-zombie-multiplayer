// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/AI/BotStateManager.h"
#include "Game/AI/BotRole.h"
#include "Game/AI/BotAIController.h"
#include "Game/Framework/MyPlayerState.h"
#include "Game/Subsystems/ActorManager.h"
#include "Engine/TriggerBox.h"
#include "Game/Characters/BaseCharacter.h"
#include "Engine/TargetPoint.h"

BotStateManager::BotStateManager()
{
}

BotStateManager::~BotStateManager()
{
}

void BotStateManager::Initialize(AActorManager* ActorMgr)
{
	UE_LOG(LogTemp, Warning, TEXT("BotStateManager: Initialized"));
	ActorManager = ActorMgr;
}

void BotStateManager::AddBot(ABotAIController* NewBot)
{
	ManagedBots.Add(NewBot);
}

void BotStateManager::OnSpikePlanted(AActor* SpikeActor)
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
		if (!PS) continue;

		ABaseCharacter* BotCharacter = Cast<ABaseCharacter>(Bot->GetPawn());
		if (!BotCharacter || BotCharacter->IsDead()) continue;

		bool IsAttacker = (PS->GetTeamId() == ETeamId::Attacker);
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
		ABotAIController* SpikeDefuserBot = Defenders[0];
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

void BotStateManager::OnSpikeDropped(AActor* SpikeActor)
{
	TArray<ABotAIController*> Defenders;
	for (ABotAIController* Bot : ManagedBots)
	{
		AMyPlayerState* PS = Bot->GetPlayerState<AMyPlayerState>();
		if (!PS) continue;

		ABaseCharacter* BotCharacter = Cast<ABaseCharacter>(Bot->GetPawn());
		if (!BotCharacter || BotCharacter->IsDead()) continue;

		Bot->SetSpikeRole(EBotRole::A_FindSpike);
		Bot->SetSpikeActor(SpikeActor);
		break;
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

void BotStateManager::OnStartRound(AActor* SpikeActor)
{
	if (!ActorManager) return;
	FName BombSite = FMath::RandBool() ? FName(TEXT("A")) : FName(TEXT("B"));
	FVector PlantLocation =
		(BombSite == FName(TEXT("A")))
		? ActorManager->GetAreaBombA()->GetActorLocation()
		: ActorManager->GetAreaBombB()->GetActorLocation();

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
		bool IsAttacker = (PS->GetTeamId() == ETeamId::Attacker);

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
		SpikeCarrierBot->SetSpikeRole(EBotRole::A_FindSpike);
	}
}

void BotStateManager::OnStartRoundZombieMode() {
	if (!ActorManager) return;
	for (ABotAIController* Bot : ManagedBots)
	{
		
	}
}
void BotStateManager::SetMatchMode(EMatchMode NewMode)
{
	CurrentMatchMode = NewMode;
}

void BotStateManager::NotifyCharacterRole(ABotAIController* Bot, ECharacterRole NewRole) {
	if (!Bot) return;
	Bot->SetCharacterRole(NewRole);
}