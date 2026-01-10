// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Bot/BotRole.h"
#include "Game/ShooterGameState.h"
#include "Characters/BaseCharacter.h"

class ABotAIController;
class AActorManager;

/**
 * 
 */
class FPSDEMO_API BotStateManager
{
public:
	BotStateManager();
	~BotStateManager();
	void Initialize(AActorManager* ActorMgr);
	void AddBot(ABotAIController* NewBot);
	void RemoveBot(ABotAIController* BotToRemove);
	void OnSpikePlanted(AActor* SpikeActor);
	void OnSpikeDefused();
	void OnSpikePickedUp(ABaseCharacter* Player);
	void OnSpikeDropped();
	void OnSpikeCarrierKilled();
	void OnStartRound(AActor* SpikeActor);
	void OnStartRoundZombieMode();
	void NotifyCharacterRole(ABotAIController* Bot, ECharacterRole NewRole);
	TArray<ABotAIController*> GetManagedBots() const {
		return ManagedBots;
	}
	void SetMatchMode(EMatchMode NewMode);
private:
	TArray<ABotAIController*> ManagedBots;
	EMatchMode CurrentMatchMode = EMatchMode::Spike;
	AActorManager* ActorManager = nullptr;
};
