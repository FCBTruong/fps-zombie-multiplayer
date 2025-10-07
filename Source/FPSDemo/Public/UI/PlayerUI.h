// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "PlayerUI.generated.h"

/**
 * 
 */
UCLASS()
class FPSDEMO_API UPlayerUI : public UUserWidget
{
	GENERATED_BODY()
public:
	void ShowPickupMessage(FString Message);
	void HidePickupMessage();
};
