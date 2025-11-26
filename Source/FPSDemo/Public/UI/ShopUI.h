// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/ShopSlotUI.h"
#include "ShopUI.generated.h"

/**
 * 
 */
UCLASS()
class FPSDEMO_API UShopUI : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* MyMoneyLb;
	TArray<UShopSlotUI*> Slots;
public:
	void OnActive();
	void NativeConstruct() override;
};
