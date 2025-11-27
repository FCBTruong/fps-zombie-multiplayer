// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ShopSlotUI.h"
#include "Controllers/MyPlayerController.h"

void UShopSlotUI::NativeConstruct()
{
	Super::NativeConstruct();
	if (Data)
	{
		if (PriceLb) {
			PriceLb->SetText(
				FText::Format(FText::FromString(TEXT("{0}$")), FText::AsNumber(Data->Price))
			);
		}

		if (IconImg) {
			IconImg->SetBrushFromTexture(Data->Icon);
		}

		if (NameLb) {
			NameLb->SetText(Data->DisplayName);
		}
	}

	if (SttLb) {
		SttLb->SetText(FText::AsNumber(SlotIndex));
	}
}

void UShopSlotUI::OnClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("Clicked on shop slot %d"), SlotIndex);

	if (!bCanBuy)
	{
		UE_LOG(LogTemp, Warning, TEXT("Cannot buy item %s, not enough money or already owned."), *Data->GetName());
		return;
	}

	AMyPlayerController* PC = Cast<AMyPlayerController>(GetWorld()->GetFirstPlayerController());
	if (PC)
	{
		double Time = FPlatformTime::Seconds();

		UE_LOG(LogTemp, Warning, TEXT("DebugTime - CLICK: %.3f"), Time);

		PC->ServerBuyItem(Data->Id);
	}
}

void UShopSlotUI::SetCanBuy(bool CanBuy)
{
	FLinearColor InactiveColor = FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("7F7F7FFF")));
	FLinearColor ActiveColor = FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("EBBE57FF")));

	this->bCanBuy = CanBuy;

	if (CanBuy)
	{
		IconImg->SetColorAndOpacity(ActiveColor);
		PriceLb->SetColorAndOpacity(ActiveColor);
		NameLb->SetColorAndOpacity(ActiveColor);
	}
	else
	{
		IconImg->SetColorAndOpacity(InactiveColor);
		PriceLb->SetColorAndOpacity(InactiveColor);
		NameLb->SetColorAndOpacity(InactiveColor);
	}
}