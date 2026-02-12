// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Shared/Types/ItemId.h"
#include "GrenadeNodeUI.generated.h"

/**
 * 
 */
UCLASS()
class FPSDEMO_API UGrenadeNodeUI : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	UImage* Icon;
	UPROPERTY(meta = (BindWidget))
	UImage* Dot;
	EItemId CurItemId = EItemId::NONE;

	void UpdateIcon(EItemId ItemId);
	void SetSelected(bool bIsSelected);
};
