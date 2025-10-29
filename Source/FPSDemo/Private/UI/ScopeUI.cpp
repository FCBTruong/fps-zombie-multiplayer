// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ScopeUI.h"

void UScopeUI::ShowScope()
{
	if (ScopeImage)
	{
		ScopeImage->SetVisibility(ESlateVisibility::Visible);
	}
	if (Crosshair)
	{
		UE_LOG(LogTemp, Warning, TEXT("Hiding Crosshair"));
		Crosshair->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UScopeUI::HideScope()
{
	if (ScopeImage)
	{
		ScopeImage->SetVisibility(ESlateVisibility::Hidden);
	}
	if (Crosshair)
	{
		UE_LOG(LogTemp, Warning, TEXT("Showing Crosshair"));
		Crosshair->SetVisibility(ESlateVisibility::Visible);
	}
}