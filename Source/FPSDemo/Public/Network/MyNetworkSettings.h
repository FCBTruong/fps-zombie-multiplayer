// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "MyNetworkSettings.generated.h"

/**
 * 
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Network Settings"))
class FPSDEMO_API UMyNetworkSettings : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:
	UPROPERTY(Config, EditAnywhere, Category = "Network")
	FString DevWebSocketUrl;

	UPROPERTY(Config, EditAnywhere, Category = "Network")
	FString ProdWebSocketUrl;
};
