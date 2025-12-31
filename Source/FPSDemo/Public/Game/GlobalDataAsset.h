// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "GlobalDataAsset.generated.h"
class UPlayerUI;
class UBehaviorTree;
/**
 * 
 */
UCLASS()
class FPSDEMO_API UGlobalDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UStaticMesh* TrajectoryMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UMaterialInterface* TrajectoryMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UPlayerUI> PlayerUIClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UNiagaraSystem* BulletTrailNS;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound")
	USoundBase* TouchSound;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	TObjectPtr<UBehaviorTree> BotBehaviorTree;
};
