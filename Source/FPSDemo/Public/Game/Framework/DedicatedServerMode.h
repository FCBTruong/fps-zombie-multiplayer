// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "DedicatedServerMode.generated.h"

/**
 * 
 */
UCLASS()
class FPSDEMO_API ADedicatedServerMode : public AGameMode
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
};
