// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ShopUI.h"
#include "Blueprint/WidgetTree.h"

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

	int MyMoney = 1000; // This should be fetched from the player's data

	for (UShopSlotUI* ShopSlot: Slots)
	{
		if (ShopSlot && ShopSlot->Data)
		{
			bool bCanAfford = MyMoney >= ShopSlot->Data->Price;
			ShopSlot->SetCanBuy(bCanAfford);
		}
	}
}