// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Game/ShooterGameState.h"
#include "TeamEliminationState.generated.h"

/**
 * 
 */
UCLASS()
class FPSDEMO_API ATeamEliminationState : public AShooterGameState
{
	GENERATED_BODY()
	
public:
	
protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;


};
