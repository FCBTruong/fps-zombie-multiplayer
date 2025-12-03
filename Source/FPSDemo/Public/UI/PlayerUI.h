// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/Image.h"
#include "Components/StackBox.h"
#include "UI/KillNotifySlot.h"
#include "UI/ShopUI.h"
#include "UI/GrenadeNodeUI.h"
#include "UI/ScopeUI.h"
#include "UI/Crosshair.h"
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
	UPROPERTY(meta = (BindWidget))
	UImage* FlashScreen;
	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* FlashScreenAnim;
	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* ShowWeaponNumbers;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* ShowWeaponIcons;

	UPROPERTY(meta = (BindWidget))
	UStackBox* GrenadesStack;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* GrenadeTitle;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* HpValueLb;

	UPROPERTY(meta = (BindWidget))
	UImage* RifleIcon;

	UPROPERTY(meta = (BindWidget))
	UImage* PistolIcon;

	TArray<UGrenadeNodeUI*> Grenades;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grenades")
	TSubclassOf<UGrenadeNodeUI> GrenadeNodeClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UKillNotifySlot> KillNotifyWidgetClass;

	TArray<UWidget*> WeaponTextNumbers;

	UPROPERTY(meta = (BindWidget))
	UCrosshair* WBP_Crosshair;

	UPROPERTY(meta = (BindWidget))
	UWidget* Pistol;

	UPROPERTY(meta = (BindWidget))
	UWidget* Rifle;
public:
	UPROPERTY(meta = (BindWidget), Transient)
	UShopUI* WBP_Shop;

	UPROPERTY(meta = (BindWidget))
	UImage* ViewmodelOverlay;

	UPROPERTY(meta = (BindWidget))
	UScopeUI* ScopeUI;

	void NativeConstruct() override;
	void ShowPickupMessage(const FString& Message);
	void HidePickupMessage();
	void UpdateHealth(float CurrentHealth, float MaxHealth);
	void UpdateAmmo(int CurrentAmmoValue, int TotalAmmoValue);
	void UpdateTeamScores(int MyTeamPoints, int OpponentTeamPoints);
	void OnUpdateScore();
	void OnHit();
	void OnEnter();
	void NotifyKill(const FString& KillerName, const FString& VictimName, UWeaponData* WeaponTex, bool bIsHeadShot);
	void ApplyFlashEffect(const float& Strength);
	void FadeOutFlashEffect();
	void OpenShop();
	void CloseShop();
	void ShowIconGrenade(EItemId ItemId, bool bShow);
	void CreateGrenadeNodes();
	void UpdateGrenades(const TArray<EItemId>& GrenadeIds);
	void UpdateCurrentWeapon(const EItemId& CurrentWeaponId);
	void ShowWeaponGuide();
	void ShowScope();
	void HideScope();
	void UpdatePistol(const EItemId& ItemId);
	void UpdateRifle(const EItemId& ItemId);
};
