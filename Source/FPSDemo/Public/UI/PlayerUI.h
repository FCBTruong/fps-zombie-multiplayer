// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/Image.h"
#include "Components/StackBox.h"
#include "UI/KillNotifySlot.h"
#include "PlayerUI.generated.h"

/**
 * 
 */
UCLASS()
class FPSDEMO_API UPlayerUI : public UUserWidget
{
	GENERATED_BODY()

protected:
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
	UPROPERTY(meta = (BindWidget))
	UImage* BloodScreen;
	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* GetHitAnim;
	UPROPERTY(meta = (BindWidget))
	UStackBox* KillNotifyStack;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UKillNotifySlot> KillNotifyWidgetClass;
public:
	void NativeConstruct() override;
	void ShowPickupMessage(const FString& Message);
	void HidePickupMessage();
	void UpdateHealth(float CurrentHealth, float MaxHealth);
	void UpdateAmmo(int CurrentAmmoValue, int TotalAmmoValue);
	void UpdateTeamScores(int MyTeamPoints, int OpponentTeamPoints);
	void OnUpdateScore();
	void OnHit();
	void OnEnter();
	void NotifyKill(const FString& KillerName, const FString& VictimName, UTexture2D* WeaponTex, bool bIsHeadShot);
};
