// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Bot/BotRole.h"

class ABotAIController;
class AActorManager;
class ABaseCharacter;

/**
 * 
 */
class FPSDEMO_API BotStateManager
{
public:
	BotStateManager();
	~BotStateManager();

	void AddBot(ABotAIController* NewBot);
	void RemoveBot(ABotAIController* BotToRemove);
	void OnSpikePlanted(FName AttackerTeamId, AActor* SpikeActor);
	void OnSpikeDefused();
	void OnSpikePickedUp(ABaseCharacter* Player);
	void OnSpikeDropped();
	void OnSpikeCarrierKilled();
	void OnStartRound(AActorManager* ActorMgr, FName AttackerTeamId, AActor* SpikeActor);
	TArray<ABotAIController*> GetManagedBots() const {
		return ManagedBots;
	}

private:
	TArray<ABotAIController*> ManagedBots;

	void AssignBotAsRole(ABotAIController* Bot, EBotRole Role);
};
