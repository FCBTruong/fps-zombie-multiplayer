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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Helicopter")
	TSubclassOf<AActor> HelicopterClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UPlayerUI> PlayerUIClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Sound")
	USoundBase* TouchSound;

	UPROPERTY(EditDefaultsOnly, Category = "AI")
	TObjectPtr<UBehaviorTree> BotBehaviorTree;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TObjectPtr<UTexture2D> KillMarkNormalIcon;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TObjectPtr<UTexture2D> KillMarkHeadshotIcon;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TObjectPtr<UTexture2D> DefenderIcon;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TObjectPtr<UTexture2D> AttackerIcon;

	UPROPERTY(EditAnywhere, Category = "Sound")
	TObjectPtr<USoundBase> KillMarkSound;

	UPROPERTY(EditAnywhere, Category = "Sound")
	TObjectPtr<USoundBase> KillHeadshotSound;

	UPROPERTY(EditAnywhere, Category = "Sound")
	TObjectPtr<USoundBase> RoundStartSound;

	UPROPERTY(EditAnywhere, Category = "Sound")
	TObjectPtr<USoundBase> SwitchingSideVoice;

	UPROPERTY(EditAnywhere, Category = "Sound")
	TObjectPtr<USoundBase> HeroPhaseActiveSound;

	UPROPERTY(EditAnywhere, Category = "Sound")
	TObjectPtr<USoundBase> GameEndSound;

	UPROPERTY(EditAnywhere, Category = "Sound")
	TObjectPtr<USoundBase> PickupSound;

	UPROPERTY(EditAnywhere, Category = "Sound")
	TObjectPtr<USoundBase> RoundEndSound;

	UPROPERTY(EditAnywhere, Category = "Sound")
	TObjectPtr<USoundBase> ZombieWinVoice;

	UPROPERTY(EditAnywhere, Category = "Sound")
	TObjectPtr<USoundBase> SoldierWinVoice;

	UPROPERTY(EditAnywhere, Category = "Sound")
	TObjectPtr<USoundBase> MeleeImpactBodySound;

	UPROPERTY(EditAnywhere, Category = "Sound")
	TObjectPtr<USoundBase> CountdownTenSound;
	UPROPERTY(EditAnywhere, Category = "Sound")
	TObjectPtr<USoundBase>	ZoomScopeSound;
	UPROPERTY(EditAnywhere, Category = "Sound")
	TObjectPtr<USoundBase>	AirdropCrateSpawnSound;

	UPROPERTY(EditAnywhere, Category = "Sound")
	TObjectPtr<USoundBase>	FireInTheHoleSound;

	UPROPERTY(EditAnywhere, Category = "Sound")
	TObjectPtr<USoundBase> NotificationSound;

	UPROPERTY(EditAnywhere, Category = "Fx")
	TObjectPtr<UParticleSystem> MeleeHitFx;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UPopupDialogUI> PopupDialogUIClass;

	UPROPERTY(EditAnywhere, Category = "Crates")
	TObjectPtr<UStaticMesh> CrateMesh;
};
