// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "GlobalDataAsset.generated.h"
class UPlayerUI;
class UBehaviorTree;
class UPopupDialogUI;
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

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TObjectPtr<UTexture2D> KillMarkNormalIcon;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TObjectPtr<UTexture2D> KillMarkHeadshotIcon;

	UPROPERTY(EditAnywhere, Category = "Sound")
	TObjectPtr<USoundBase> KillMarkSound;

	UPROPERTY(EditAnywhere, Category = "Sound")
	TObjectPtr<USoundBase> KillHeadshotSound;

	UPROPERTY(EditAnywhere, Category = "Sound")
	TObjectPtr<USoundBase> MeleeImpactBodySound;

	UPROPERTY(EditAnywhere, Category = "Sound")
	TObjectPtr<USoundBase> CountdownTenSound;

	UPROPERTY(EditAnywhere, Category = "Fx")
	TObjectPtr<UParticleSystem> MeleeHitFx;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UPopupDialogUI> PopupDialogUIClass;
};
