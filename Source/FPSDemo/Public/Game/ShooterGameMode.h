// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/WeaponBase.h"
#include "GameFramework/GameMode.h"
#include "ShooterGameMode.generated.h"

/**
 * 
 */
UCLASS()
class FPSDEMO_API AShooterGameMode : public AGameMode
{
	GENERATED_BODY()
public:
	virtual void StartPlay() override;
	virtual void NotifyPlayerKilled(class AController* Killer, class AController* Victim, class AActor* DamageCauser = nullptr);
protected:
    // Configurable in editor
    UPROPERTY(EditDefaultsOnly, Category = "Weapons")
    TArray<TSubclassOf<AWeaponBase>> WeaponClasses;
    virtual void PostLogin(APlayerController* NewPlayer) override;
  
};
