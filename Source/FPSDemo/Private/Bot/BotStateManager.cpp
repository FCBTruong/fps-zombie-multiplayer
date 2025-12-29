// Fill out your copyright notice in the Description page of Project Settings.


#include "Bot/BotStateManager.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Bot/BotRole.h"
#include "Controllers/BotAIController.h"
#include "Controllers/MyPlayerState.h"
#include "Game/ShooterGameState.h"
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
		UBlackboardComponent* BB = Bot->GetBlackboardComponent();
		if (!BB) continue;

		AMyPlayerState* PS = Bot->GetPlayerState<AMyPlayerState>();
		if (PS->IsAlive() == false) continue;

		bool IsAttacker = (PS->GetTeamID() == AttackerTeamId);
		if (IsAttacker) {
			const int32 Index = FMath::RandRange(0, UE_ARRAY_COUNT(AttackerRoles) - 1);
			const EBotRole ChosenRole = AttackerRoles[Index];
			UE_LOG(LogTemp, Warning, TEXT("Spike planted Bot %s assigned as role %d"), *Bot->GetName(), static_cast<uint8>(ChosenRole));
			BB->SetValueAsEnum(TEXT("E_Role"), static_cast<uint8>(ChosenRole));
		}
		else {
			const int32 Index = FMath::RandRange(0, UE_ARRAY_COUNT(DefenderRoles) - 1);
			const EBotRole ChosenRole = DefenderRoles[Index];
			BB->SetValueAsEnum(TEXT("E_Role"), static_cast<uint8>(ChosenRole));
			Defenders.Add(Bot);
		}
		BB->SetValueAsObject(TEXT("Obj_SpikeActor"), SpikeActor);
	}
	// assign one defender as spike defuser
	if (Defenders.Num() > 0)
	{
		int32 RandomIndex = FMath::RandRange(0, Defenders.Num() - 1);
		ABotAIController* SpikeDefuserBot = Defenders[RandomIndex];
		SpikeDefuserBot->GetBlackboardComponent()->SetValueAsEnum(TEXT("E_Role"), static_cast<uint8>(EBotRole::D_Defuser));
		UE_LOG(LogTemp, Warning, TEXT("Bot %s assigned as Spike Defuser"), *SpikeDefuserBot->GetName());
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
			UBlackboardComponent* BB = Bot->GetBlackboardComponent();
			if (!BB) continue;
			EBotRole CurrentRole = static_cast<EBotRole>(BB->GetValueAsEnum(TEXT("E_Role")));
			if (CurrentRole == EBotRole::A_FindSpike) {
				BB->SetValueAsEnum(TEXT("E_Role"), static_cast<uint8>(EBotRole::Scout));
			}
		}
	}
	if (!PlayerBot) return;

	UBlackboardComponent* BB = PlayerBot->GetBlackboardComponent();
	if (!BB) return;

	BB->SetValueAsEnum(TEXT("E_Role"), static_cast<uint8>(EBotRole::A_Carrier));
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
		UE_LOG(LogTemp, Warning, TEXT("Deubg: Bot %s is %s"), *Bot->GetName(), IsAttacker ? TEXT("Attacker") : TEXT("Defender"));
		// update bot data to blackboard
		UBlackboardComponent* BB = Bot->GetBlackboardComponent();
		if (!BB) continue;

		UE_LOG(LogTemp, Warning, TEXT("Debug: Updating bot %s blackboard"), *Bot->GetName());

		BB->SetValueAsBool(TEXT("B_IsAttacker"), IsAttacker);
		BB->SetValueAsName(TEXT("Name_BombSite"), BombSite);

		BB->SetValueAsVector(TEXT("Vec_PlantLocation"), PlantLocation);

		if (IsAttacker)
		{
			const int32 Index = FMath::RandRange(0, UE_ARRAY_COUNT(AttackerRoles) - 1);
			const EBotRole ChosenRole = AttackerRoles[Index];
			BB->SetValueAsEnum(TEXT("E_Role"), static_cast<uint8>(ChosenRole));
			Attackers.Add(Bot);
		}
		else {
			const int32 Index = FMath::RandRange(0, UE_ARRAY_COUNT(DefenderRoles) - 1);
			const EBotRole ChosenRole = DefenderRoles[Index];
			BB->SetValueAsEnum(TEXT("E_Role"), static_cast<uint8>(ChosenRole));
		}
		BB->SetValueAsObject(TEXT("Obj_SpikeActor"), SpikeActor);
	}

	// auto assign spike carrier for one of the attackers
	if (Attackers.Num() > 0)
	{
		int32 RandomIndex = FMath::RandRange(0, Attackers.Num() - 1);
		ABotAIController* SpikeCarrierBot = Attackers[RandomIndex];
		SpikeCarrierBot->GetBlackboardComponent()->SetValueAsEnum(TEXT("E_Role"), static_cast<uint8>(EBotRole::A_FindSpike));
		UE_LOG(LogTemp, Warning, TEXT("Bot %s assigned as Spike Carrier"), *SpikeCarrierBot->GetName());
	}
}

void BotStateManager::AssignBotAsRole(ABotAIController* Bot, EBotRole NewRole)
{
	if (!Bot) return;
	UBlackboardComponent* BB = Bot->GetBlackboardComponent();
	if (!BB) return;
	BB->SetValueAsEnum(TEXT("E_Role"), static_cast<uint8>(NewRole));
}