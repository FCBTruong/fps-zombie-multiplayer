// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Slider.h"
#include "GameSettingsUI.generated.h"

/**
 * 
 */
UCLASS()
class FPSDEMO_API UGameSettingsUI : public UUserWidget
{
	GENERATED_BODY()

protected:
	void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	USlider* MouseSlider;

	UFUNCTION()
	void OnMouseSliderCaptureEnd();
};
