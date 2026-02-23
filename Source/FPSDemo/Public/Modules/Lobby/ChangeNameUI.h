// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/EditableTextBox.h"
#include "Components/Button.h"
#include "ChangeNameUI.generated.h"

UCLASS()
class FPSDEMO_API UChangeNameUI : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* InputName;

	UPROPERTY(meta = (BindWidget))
	UButton* BtnOk;

	// Called when the OK button is clicked
	UFUNCTION()
	void OnClickOk();

	// Called whenever the input text changes
	UFUNCTION()
	void OnNameTextChanged(const FText& NewText);

private:
	// Update button enabled state based on current input
	void UpdateOkButtonState();
};