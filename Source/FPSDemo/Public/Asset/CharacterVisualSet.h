// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CharacterVisualSet.generated.h"

/**
 * 
 */
UCLASS()
class FPSDEMO_API UCharacterVisualSet : public UDataAsset
{
	GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TObjectPtr<USkeletalMesh> TpsMesh = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TObjectPtr<USkeletalMesh> FpsMesh = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TSubclassOf<UAnimInstance> TpsAnimClass;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TSubclassOf<UAnimInstance> FpsAnimClass;
};
