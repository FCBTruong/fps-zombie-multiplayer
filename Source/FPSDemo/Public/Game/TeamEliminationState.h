// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Game/ShooterGameState.h"
#include "TeamEliminationState.generated.h"

DECLARE_MULTICAST_DELEGATE(OnUpdateScore)
/**
 * 
 */
UCLASS()
class FPSDEMO_API ATeamEliminationState : public AShooterGameState
{
	GENERATED_BODY()
	
public:
	UPROPERTY(ReplicatedUsing = OnRep_Score)
	int TeamAScore = 0;

	UPROPERTY(ReplicatedUsing = OnRep_Score)
	int TeamBScore = 0;

	OnUpdateScore OnUpdateScore;
protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION()
	void OnRep_Score();
};
