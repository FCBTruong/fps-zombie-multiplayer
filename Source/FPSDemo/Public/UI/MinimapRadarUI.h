// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Image.h"
#include "Blueprint/UserWidget.h"
#include "MinimapRadarUI.generated.h"

/**
 * 
 */
UCLASS()
class FPSDEMO_API UMinimapRadarUI : public UUserWidget
{
	GENERATED_BODY()

protected:

	UPROPERTY(meta = (BindWidget))
	class UImage* MinimapImage;

	UPROPERTY(meta = (BindWidget))
	class UImage* Dot;

	FVector WorldOrigin;
	FVector WorldExtent;
	FVector PlaneSize;
	FVector2D MinimapSize;
public:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual void NativeConstruct() override;
};
