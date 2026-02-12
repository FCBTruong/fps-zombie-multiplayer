// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Math/Color.h"
#include "Crosshair.generated.h"


struct FUECrosshairData
{
    // Inner
    float InnerThickness = 0.f;
    float InnerLength = 0.f;
    float InnerOffset = 0.f;
    float InnerAlpha = 1.f;
    bool  InnerFiringError = false;

    // Outer
    float OuterThickness = 0.f;
    float OuterLength = 0.f;
    float OuterOffset = 0.f;
    float OuterAlpha = 1.f;
    bool  OuterFiringError = false;

    // Dot
    bool DotEnable = false;

    // General
    FLinearColor CrosshairColor = FLinearColor::White;
    bool OutlineEnabled = false;
    float OutlineThickness = 0.f;
    float OutlineOpacity = 0.f;
};


/**
 * 
 */
UCLASS()
class FPSDEMO_API UCrosshair : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget)) UImage* InnerTop;
	UPROPERTY(meta = (BindWidget)) UImage* InnerBottom;
	UPROPERTY(meta = (BindWidget)) UImage* InnerLeft;
	UPROPERTY(meta = (BindWidget)) UImage* InnerRight;

	UPROPERTY(meta = (BindWidget)) UImage* OuterTop;
	UPROPERTY(meta = (BindWidget)) UImage* OuterBottom;
	UPROPERTY(meta = (BindWidget)) UImage* OuterLeft;
	UPROPERTY(meta = (BindWidget)) UImage* OuterRight;

	UPROPERTY(meta = (BindWidget)) UImage* Dot;

    FUECrosshairData ParseCrosshairCode(const FString& Code);
    void ApplyData(const FUECrosshairData& D);
    void NativeConstruct() override;

	const FString CrosshairCodeDefault = "0;P;h;0;d;1;z;1;a;0;0t;3;0l;14;0o;6;0a;1;0f;0;1b;0";

    void SetCrosshairCode(const FString& Code);
};
