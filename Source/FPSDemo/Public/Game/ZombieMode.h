// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Game/ShooterGameMode.h"
#include "ZombieMode.generated.h"

/**
 * 
 */
UCLASS()
class FPSDEMO_API AZombieMode : public AShooterGameMode
{
	GENERATED_BODY()
	
protected:
	virtual void StartPlay() override;
	virtual void StartRound() override;
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
	void EndRound(FName WinningTeam);
	void AssignZombieRoles();

	FTimerHandle RoleAssignTimerHandle;
public:
	virtual void NotifyPlayerKilled(class AController* Killer, class AController* Victim, const UItemConfig* DamageCauser, bool bWasHeadShot) override;
};
