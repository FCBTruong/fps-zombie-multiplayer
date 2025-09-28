// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "WeaponBase.h"
#include "Net/UnrealNetwork.h"
#include "ShooterGameState.generated.h"

/**
 * 
 */
UCLASS()
class FPSDEMO_API AShooterGameState : public AGameState
{
	GENERATED_BODY()

public:
	UPROPERTY(Replicated)
	TArray<TObjectPtr<class AWeaponBase>> WeaponsOnMap;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
