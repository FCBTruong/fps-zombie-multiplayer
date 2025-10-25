// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/CanvasPanel.h"
#include "Components/Image.h"
#include "ScopeUI.generated.h"

/**
 * 
 */
UCLASS()
class FPSDEMO_API UScopeUI : public UUserWidget
{
	GENERATED_BODY()

private:
	UPROPERTY(meta = (BindWidget))
	UImage* ScopeImage;

	UPROPERTY(meta = (BindWidget))
	UCanvasPanel* Crosshair;
public:
	void ShowScope();
	void HideScope();
};
