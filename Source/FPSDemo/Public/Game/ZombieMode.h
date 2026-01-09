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
	virtual APawn* SpawnDefaultPawnAtTransform_Implementation(
		AController* NewPlayer,
		const FTransform& SpawnTransform
	) override;
	virtual void EndRound(FName WinningTeam) override;
	void AssignZombieRoles();
	void BecomeZombie(AController* Controller);
	void ReviveZombie(ABaseCharacter* ZombieCharacter);

	FTimerHandle RoleAssignTimerHandle;
public:
	virtual void OnCharacterKilled(class AController* Killer, ABaseCharacter* Victim, const UItemConfig* DamageCauser, bool bWasHeadShot) override;
	virtual EMatchMode GetMatchMode() const {
		return EMatchMode::Zombie;
	}
};
