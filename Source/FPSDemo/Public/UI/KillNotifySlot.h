// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/StackBox.h"
#include "KillNotifySlot.generated.h"

/**
 * 
 */
UCLASS()
class FPSDEMO_API UKillNotifySlot : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* KillerLb;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* VictimLb;
	UPROPERTY(meta = (BindWidget))
	UImage* WeaponIcon;
	UPROPERTY(meta = (BindWidget))
	UImage* HeadShotIcon;

	void SetInfo(const FString& KillerName, const FString& VictimName, UTexture2D* WeaponTex, bool bIsHeadShot);
};
