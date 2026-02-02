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
#include "Components/VerticalBox.h"
#include "UI/Crosshair.h"
#include "Game/MyMatchState.h"
#include "Controllers/MyPlayerState.h"
#include "Data/TeamId.h"
#include "PlayerUI.generated.h"

class UItemConfig;
class UMinimapRadarUI;
/**
 * 
 */
UCLASS()
class FPSDEMO_API UPlayerUI : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* RoundLb;
	UPROPERTY(meta = (BindWidget))
	UProgressBar* HpBar;
	UPROPERTY(meta = (BindWidget))
	UProgressBar* ArmorBar;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* RemainingAmmo;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* CurrentAmmo;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* FirstTeamScore;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* SecondTeamScore;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* FirstTeamLb;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* SecondTeamLb;
	UPROPERTY(meta = (BindWidget))
	UImage* BloodScreen;
	UPROPERTY(meta = (BindWidget))
	UWidget* SwitchSideEffPn;
	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* ShowAnimSwitchSide;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* GetHitAnim;
	UPROPERTY(meta = (BindWidget))
	UStackBox* KillNotifyStack;
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
	UTextBlock* PlayerNameLb;

	UPROPERTY(meta = (BindWidget))
	UWidget* MatchInfoPn;

	UPROPERTY(meta = (BindWidget))
	UImage* RifleIcon;

	UPROPERTY(meta = (BindWidget))
	UImage* PistolIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Grenades")
	TSubclassOf<UGrenadeNodeUI> GrenadeNodeClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UKillNotifySlot> KillNotifyWidgetClass;

	UPROPERTY(meta = (BindWidget))
	UCrosshair* WBP_Crosshair;

	UPROPERTY(meta = (BindWidget))
	UWidget* Pistol;

	UPROPERTY(meta = (BindWidget))
	UWidget* Rifle;

	UPROPERTY(meta = (BindWidget))
	UImage* TeamIcon;

	UPROPERTY(meta = (BindWidget))
	UWidget* SpikeSlot;

	UPROPERTY(meta = (BindWidget))
	UVerticalBox* WeaponsBox;

	UPROPERTY(meta = (BindWidget))
	UWidget* PnSpike;
	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* StartPlantSpikeAnim;
	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* StartDefuseSpikeAnim;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* MatchToastLb;
	UPROPERTY(meta = (BindWidget))
	UWidget* MatchToastPn;

	UPROPERTY(meta = (BindWidget))
	UWidget* MatchStatePn;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* MatchStateTxt;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* ShowMatchStateAnim;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* NotiToastLb;
	UPROPERTY(meta = (BindWidget))
	UWidget* NotiToastPn;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* MatchTimeLb;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* ShowNotiToastAnim;

	UPROPERTY(meta = (BindWidget))
	UWidget* PhotonPlantedIcon;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* AmmorPointLb;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* PhotonPlantedAnim;

	UPROPERTY(meta = (BindWidget))
	UImage* KillMarkIcon;

	UPROPERTY(meta = (BindWidget))
	UImage* CurrentItemIcon;

	UPROPERTY(meta = (BindWidget))
	UWidget* AmmoPn;

	UPROPERTY(meta = (BindWidget))
	UWidget* ZombieVsHeroPn;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* HeroNumLb;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ZombieNumLb;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* KillMarkAnim;

	int RoundTimeEnd = 0;
	bool bPlayedTenSec = false;
	TArray<UGrenadeNodeUI*> Grenades;
	TArray<UWidget*> WeaponTextNumbers;
	FTimerHandle RoundClockTimerHandle;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	void DoShowMatchStateToast(FText Txt);
	void StartRoundClock();
	void StopRoundClock();
	void UpdateRoundClockOnce();
public:
	UPROPERTY(meta = (BindWidget), Transient)
	UShopUI* WBP_Shop;

	UPROPERTY(meta = (BindWidget))
	UScopeUI* ScopeUI;

	UPROPERTY(meta = (BindWidget))
	UMinimapRadarUI* RadarWidget;

	void NativeConstruct() override;
	void ShowPickupMessage(const FString& Message);
	void HidePickupMessage();
	void UpdateHealth(float CurrentHealth, float MaxHealth);
	void UpdateAmmo(int CurrentAmmoValue, int RemainAmmoValue);
	void UpdateTeamScores(int FirstScore, int SecondScore);
	void OnHit();
	void OnEnter();
	void NotifyKill(const AMyPlayerState* Killer, const AMyPlayerState* Victem, const UItemConfig* WeaponTex, bool bIsHeadShot);
	void OpenShop();
	void CloseShop();
	void ShowIconGrenade(EItemId ItemId, bool bShow);
	void CreateGrenadeNodes();
	void UpdateGrenades(const TArray<EItemId>& GrenadeIds);
	void UpdateCurrentWeapon(EItemId CurrentWeaponId);
	void UpdateSpikeSlot(bool bHasSpike);
	void ShowWeaponGuide();
	void ShowScope();
	void HideScope();
	void UpdatePistol(const EItemId& ItemId);
	void UpdateRifle(const EItemId& ItemId);
	void OnUpdatePlantSpikeState(bool IsPlanting);
	void OnUpdateDefuseSpikeState(bool IsDefusing);
	void ShowMatchStateToast(FText Txt, float Delay);
	void ShowNotiToast(FText Txt);
	void OnUpdateRoundTime(int TimeEnd);
	void UpdateGameState(const EMyMatchState& State);
	void ShowScoreboard(bool bShow);
	void UpdateArmor(int AmmorPoints, int MaxArmorPoints);
	void ShowKillMark(bool bHeadShot = false);
	void UpdatePlayerName(const FString& PlayerName);
	void SetRadarVisible(bool bVisible);
	void SetMatchInfoPnVisible(bool bVisible);
	void ShowGameResult(ETeamId WinningTeam);
	void OnSwitchSide();
	void UpdateTeamId(ETeamId NewTeamId);
	void UpdateRoundNumber();
	void UpdateHeroPhase();
};
