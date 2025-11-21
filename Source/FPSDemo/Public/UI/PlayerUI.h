// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "PlayerUI.generated.h"

/**
 * 
 */
UCLASS()
class FPSDEMO_API UPlayerUI : public UUserWidget
{
	GENERATED_BODY()

private:
	UPROPERTY(meta = (BindWidget))
	UProgressBar* HpBar;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* TotalAmmo;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* CurrentAmmo;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* MyTeamScore;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* OpponentTeamScore;

public:
	void NativeConstruct() override;
	void ShowPickupMessage(const FString& Message);
	void HidePickupMessage();
	void UpdateHealth(float CurrentHealth, float MaxHealth);
	void UpdateAmmo(int CurrentAmmoValue, int TotalAmmoValue);
	void UpdateTeamScores(int MyTeamPoints, int OpponentTeamPoints);
	void OnUpdateScore();
};
