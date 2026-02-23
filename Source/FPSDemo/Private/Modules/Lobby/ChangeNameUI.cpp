// Fill out your copyright notice in the Description page of Project Settings.

#include "Modules/Lobby/ChangeNameUI.h"
#include "Shared/System/PlayerInfoManager.h"

void UChangeNameUI::NativeConstruct()
{
	Super::NativeConstruct();

	// Bind button click event
	if (BtnOk)
	{
		BtnOk->OnClicked.AddDynamic(this, &UChangeNameUI::OnClickOk);
	}

	// Bind text changed event
	if (InputName)
	{
		InputName->OnTextChanged.AddDynamic(this, &UChangeNameUI::OnNameTextChanged);
	}

	// Initialize button state when widget is created
	UpdateOkButtonState();
}

void UChangeNameUI::OnNameTextChanged(const FText& NewText)
{
	UpdateOkButtonState();
}

void UChangeNameUI::UpdateOkButtonState()
{
	if (!BtnOk || !InputName)
	{
		return;
	}

	// Trim spaces before checking length
	const FString Name = InputName->GetText().ToString().TrimStartAndEnd();

	// Disable OK if length is less than 4
	const bool bCanConfirm = Name.Len() >= 4;
	BtnOk->SetIsEnabled(bCanConfirm);
}

void UChangeNameUI::OnClickOk()
{
	if (!InputName)
	{
		return;
	}

	FString Name = InputName->GetText().ToString().TrimStartAndEnd();

	// Extra safety check
	if (Name.Len() < 4)
	{
		return;
	}

	if (Name.Len() > 40)
	{
		Name = Name.Left(40);
	}

	UE_LOG(LogTemp, Log, TEXT("Player name confirmed: %s"), *Name);
	UPlayerInfoManager* PlayerInfoMgr = UPlayerInfoManager::Get(GetWorld());
	if (PlayerInfoMgr)
	{
		PlayerInfoMgr->ChangeName(Name);
	}

	// close the UI after confirming
	RemoveFromParent();
}