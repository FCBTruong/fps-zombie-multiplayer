// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "PlayerMapDot.generated.h"

/**
 * 
 */
UCLASS()
class FPSDEMO_API UPlayerMapDot : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	UImage* Dot;

	UPROPERTY(meta = (BindWidget))
	UImage* LightScanIcon;

	UPROPERTY(meta = (BindWidget))
	UWidget* Arrow;

	UPROPERTY(meta = (BindWidget))
	UWidget* DeadIcon;

	UPROPERTY(meta = (BindWidget))
	UWidget* SpikeIcon;

	void UpdateData(bool bIsMe, bool bIsDead, bool bHasSpike);
	void SetIsTeammateVisual(bool bIsTeammate);

	void NativeConstruct() override;
};
