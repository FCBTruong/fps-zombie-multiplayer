// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ScopeUI.h"

void UScopeUI::ShowScope()
{
	if (ScopeImage)
	{
		ScopeImage->SetVisibility(ESlateVisibility::Visible);
	}
}

void UScopeUI::HideScope()
{
	if (ScopeImage)
	{
		ScopeImage->SetVisibility(ESlateVisibility::Hidden);
	}
}