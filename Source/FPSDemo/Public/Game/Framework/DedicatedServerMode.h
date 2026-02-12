// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "DedicatedServerMode.generated.h"

struct FProcessParameters;
/**
 * 
 */
UCLASS()
class FPSDEMO_API ADedicatedServerMode : public AGameMode
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	
private:
	void InitGameLift();

private:
	TSharedPtr<FProcessParameters> ProcessParameters;
};
