// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PlayerUI.h"

void UPlayerUI::ShowPickupMessage(FString Message)
{
    if (UTextBlock* Label = Cast<UTextBlock>(GetWidgetFromName(TEXT("PickupLabel"))))
    {
        Label->SetText(FText::FromString(Message));
        Label->SetVisibility(ESlateVisibility::Visible);
    }
}

void UPlayerUI::HidePickupMessage()
{
    if (UTextBlock* Label = Cast<UTextBlock>(GetWidgetFromName(TEXT("PickupLabel"))))
    {
        Label->SetVisibility(ESlateVisibility::Hidden);
    }
}