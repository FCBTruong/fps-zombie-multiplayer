// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/UI/GameSettingsUI.h"
#include "Game/Framework/MyPlayerController.h"

void UGameSettingsUI::NativeConstruct()
{
	Super::NativeConstruct();
	if (MouseSlider)
	{
		MouseSlider->OnMouseCaptureEnd.AddDynamic(this, &UGameSettingsUI::OnMouseSliderCaptureEnd);
	}

	AMyPlayerController* PC = Cast<AMyPlayerController>(GetOwningPlayer());
	if (PC) {
		float CurrentSensitivity = PC->GetMouseSensitivity();
		if (MouseSlider)
		{
			MouseSlider->SetValue(CurrentSensitivity);
		}
	}
}

void UGameSettingsUI::OnMouseSliderCaptureEnd()
{
	if (MouseSlider)
	{
		float NewSensitivity = MouseSlider->GetValue();
		UE_LOG(LogTemp, Log, TEXT("New mouse sensitivity: %f"), NewSensitivity);

		AMyPlayerController* PC = Cast<AMyPlayerController>(GetOwningPlayer());
		if (PC) {
			PC->SetMouseSensitivity(NewSensitivity);
		}
	}
}