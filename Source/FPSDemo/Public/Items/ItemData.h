// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Items/ItemIds.h"
#include "ItemData.generated.h"

/**
 * 
 */
UCLASS()
class FPSDEMO_API UItemData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	EItemId Id;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	USkeletalMesh* Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	UStaticMesh* StaticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	UTexture2D* Icon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	FText DisplayName;
};
