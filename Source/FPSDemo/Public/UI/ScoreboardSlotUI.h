// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Controllers/MyPlayerState.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "ScoreboardSlotUI.generated.h"

/**
 * 
 */
UCLASS()
class FPSDEMO_API UScoreboardSlotUI : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* PlayerNameLb;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* KillLb;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* DeathLb;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* AssistLb;

	UPROPERTY(meta = (BindWidget))
	class UImage* BackgroundImg;

	UPROPERTY(meta = (BindWidget))
	class UImage* HighlightImg;

public:
	void Setup(const AMyPlayerState* PS, bool IsMe = false);
};
