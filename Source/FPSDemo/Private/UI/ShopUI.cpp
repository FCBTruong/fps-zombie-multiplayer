// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ShopUI.h"
#include "Blueprint/WidgetTree.h"
#include "Controllers/MyPlayerState.h"
#include "Utils/GameUtils.h"

void UShopUI::NativeConstruct()
{
	Super::NativeConstruct();
	// Additional initialization can be done here if needed.

	Slots = TArray<UShopSlotUI*>();
	TArray<UWidget*> AllWidgets;
	WidgetTree->GetAllWidgets(AllWidgets);

	for (UWidget* Widget : AllWidgets)
	{
		if (!Widget) continue;

		const FString& Name = Widget->GetName();

		if (Name.Contains(TEXT("ShopSlot")))
		{
			if (UShopSlotUI* SlotWidget = Cast<UShopSlotUI>(Widget))
			{
				Slots.Add(SlotWidget);
			}
		}
	}
}

void UShopUI::OnActive()
{
	// This function can be used to initialize or refresh the shop UI when it becomes active.
	UE_LOG(LogTemp, Log, TEXT("Shop UI is now active."));

	// log num slot
	UE_LOG(LogTemp, Log, TEXT("Number of shop slots: %d"), Slots.Num());

	UpdateShopMoneyStatus();
}

void UShopUI::UpdateShopMoneyStatus()
{
	UE_LOG(LogTemp, Log, TEXT("Updating shop money status."));
	AMyPlayerState* PS = GetWorld()->GetFirstPlayerController()->GetPlayerState<AMyPlayerState>();
	if (!PS)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayerState is null in ShopUI::UpdateShopMoneyStatus"));
		return;
	}
	int MyMoney = PS->GetMoney();
	MyMoneyLb->SetText(FText::FromString(TEXT("$") + GameUtils::PointNumber(MyMoney)));
	for (UShopSlotUI* ShopSlot : Slots)
	{
		if (ShopSlot && ShopSlot->Data)
		{
			bool bCanBuy = PS->CanBuyThisItem(ShopSlot->Data);
			ShopSlot->SetCanBuy(bCanBuy);
		}
	}
}

void UShopUI::UpdateBoughtItemsStatus()
{
	UE_LOG(LogTemp, Log, TEXT("Updating bought items status in shop UI."));
	AMyPlayerState* PS = GetWorld()->GetFirstPlayerController()->GetPlayerState<AMyPlayerState>();
	if (!PS)
	{
		UE_LOG(LogTemp, Warning, TEXT("PlayerState is null in ShopUI::UpdateBoughtItemsStatus"));
		return;
	}
	for (UShopSlotUI* ShopSlot : Slots)
	{
		if (ShopSlot && ShopSlot->Data)
		{
			bool bCanBuy = PS->CanBuyThisItem(ShopSlot->Data);
			ShopSlot->SetCanBuy(bCanBuy);
		}
	}
}