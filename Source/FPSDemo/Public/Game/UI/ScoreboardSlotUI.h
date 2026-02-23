// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "ScoreboardSlotUI.generated.h"

class AMyPlayerState;
/**
 * 
 */
UCLASS()
class FPSDEMO_API UScoreboardSlotUI : public UUserWidget
{
	GENERATED_BODY()

protected:
	void NativeConstruct() override;
	void NativeDestruct() override;
	void RefreshDynamicInfo();

	TWeakObjectPtr<const AMyPlayerState> CachedPS;
	FTimerHandle RefreshTimerHandle;

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

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* PingLb;
public:
	void Setup(const AMyPlayerState* PS, bool IsMe = false);
};
