// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GlobalDataAsset.generated.h"
class UPlayerUI;
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
};
