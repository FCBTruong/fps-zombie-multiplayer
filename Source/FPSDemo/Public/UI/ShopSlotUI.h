// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Weapons/WeaponData.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "ShopSlotUI.generated.h"

/**
 * 
 */
UCLASS()
class FPSDEMO_API UShopSlotUI : public UUserWidget
{
	GENERATED_BODY()

private:
	bool bCanBuy = true;

public:
	void NativeConstruct() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	UWeaponData* Data;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* PriceLb;

	UPROPERTY(meta = (BindWidget))
	UImage* IconImg;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* NameLb;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* SttLb;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Shop")
	int SlotIndex;

	UFUNCTION(BlueprintCallable)
	void OnClicked();

	void SetCanBuy(bool bCanBuy);
};
