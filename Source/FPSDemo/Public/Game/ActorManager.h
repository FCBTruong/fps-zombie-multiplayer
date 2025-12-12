// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include <Engine/TriggerBox.h>
#include <Engine/TargetPoint.h>
#include "ActorManager.generated.h"

UCLASS()
class FPSDEMO_API AActorManager : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AActorManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditInstanceOnly, Category = "Setup")
	AActor* BombAreaA;

	UPROPERTY(EditInstanceOnly, Category = "Setup")
	AActor* BombAreaB;

	UPROPERTY(EditInstanceOnly, Category = "Setup")
	ATriggerBox* TriggerBoxAreaA;

	UPROPERTY(EditInstanceOnly, Category = "Setup")
	ATriggerBox* TriggerBoxAreaB;

	UPROPERTY(EditInstanceOnly, Category = "Setup")
	ATargetPoint* TargetPointSpike;

	UPROPERTY(EditInstanceOnly, Category = "Setup")
	TArray<APlayerStart*> AttackerStarts;

	UPROPERTY(EditInstanceOnly, Category = "Setup")
	TArray<APlayerStart*> DefenderStarts;

	UPROPERTY(EditInstanceOnly, Category = "Setup")
	TArray<ATargetPoint*> HoldPointsA;

	UPROPERTY(EditInstanceOnly, Category = "Setup")
	TArray<ATargetPoint*> HoldPointsB;

	TMap<APlayerStart*, bool> StartUsage;

	APlayerStart* GetRandomStart(const TArray<APlayerStart*>& Starts);
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	ATriggerBox* GetAreaBombA() { return TriggerBoxAreaA; };
	ATriggerBox* GetAreaBombB() { return TriggerBoxAreaB; };
	FVector GetSpikeStartLocation();
	APlayerStart* GetRandomAttackerStart();
	APlayerStart* GetRandomDefenderStart();
	void ResetPlayerStartsUsage();

	UPROPERTY(EditInstanceOnly, Category = "Setup")
	AActor* MainPlane;
	static AActorManager* Get(UObject* WorldContextObject);
	FVector GetRandomHoldLocationNearBombSite(FName BombSiteName);
};
