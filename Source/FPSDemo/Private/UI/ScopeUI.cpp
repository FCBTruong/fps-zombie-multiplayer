// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ScopeUI.h"

void UScopeUI::ShowScope()
{
	this->SetVisibility(ESlateVisibility::Visible);
}

void UScopeUI::HideScope()
{
	this->SetVisibility(ESlateVisibility::Hidden);
}