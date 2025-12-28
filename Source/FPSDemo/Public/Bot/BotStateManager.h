// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

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

	void AddBot(ABotAIController* NewBot);
	void RemoveBot(ABotAIController* BotToRemove);
	void OnSpikePlanted(FName AttackerTeamId, AActor* SpikeActor);
	void OnSpikeDefused();
	void OnSpikePickedUp();
	void OnSpikeDropped();
	void OnSpikeCarrierKilled();
	void OnStartRound(AActorManager* ActorMgr, FName AttackerTeamId);

private:
	TArray<ABotAIController*> ManagedBots;
};
