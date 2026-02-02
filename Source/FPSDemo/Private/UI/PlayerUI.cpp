// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PlayerUI.h"
#include "Game/TeamEliminationState.h"
#include "Game/GameManager.h"
#include "UI/ScoreboardUI.h"
#include "Items/ItemConfig.h"
#include "Game/ItemsManager.h"
#include "Items/FirearmConfig.h"
#include "Game/GlobalDataAsset.h"
#include <Kismet/GameplayStatics.h>
#include "UI/MinimapRadarUI.h"
#include <Components/CanvasPanelSlot.h>

void UPlayerUI::NativeConstruct()
{
    Super::NativeConstruct();
    if (HpBar)
    {
        HpBar->SetPercent(1.0f);
    }
	CreateGrenadeNodes();

    for (int32 i = 1; i <= 4; i++)
    {
        const FName WidgetName(*FString::Printf(TEXT("NumberWeapon%d"), i));
        if (UWidget* Widget = GetWidgetFromName(WidgetName))
        {
            WeaponTextNumbers.Add(Widget);
        }
    }

    // update UI by game mode
	AShooterGameState* GS = GetWorld() ? GetWorld()->GetGameState<AShooterGameState>() : nullptr;
	if (GS)
    {
		EMatchMode CurrentMatchMode = GS->GetMatchMode();
        if (CurrentMatchMode == EMatchMode::Zombie)
        {
			RoundLb->SetVisibility(ESlateVisibility::Visible);
            if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(MatchTimeLb->Slot))
            {
                FVector2D Pos = CanvasSlot->GetPosition();
                Pos.Y += 10.f;
                CanvasSlot->SetPosition(Pos);
            }
            if (FirstTeamLb)
            {
                FirstTeamLb->SetText(FText::FromString(TEXT("Soldier")));
            }
            if (SecondTeamLb)
            {
                SecondTeamLb->SetText(FText::FromString(TEXT("Zombie")));
            }
        }
        else if (CurrentMatchMode == EMatchMode::Spike)
        {
			RoundLb->SetVisibility(ESlateVisibility::Hidden);
            if (FirstTeamLb)
            {
                FirstTeamLb->SetText(FText::FromString(TEXT("Attacker")));
            }
            if (SecondTeamLb)
            {
                SecondTeamLb->SetText(FText::FromString(TEXT("Defender")));
            }
        }
        else // TeamDeathMatch
        {
            RoundLb->SetVisibility(ESlateVisibility::Hidden);
            if (FirstTeamLb)
            {
                FirstTeamLb->SetText(FText::FromString(TEXT("")));
            }
            if (SecondTeamLb)
            {
                SecondTeamLb->SetText(FText::FromString(TEXT("")));
            }
        }
	}
}

void UPlayerUI::NativeTick(const FGeometry& MyGeometry, float InDeltaTime) {
	Super::NativeTick(MyGeometry, InDeltaTime);
}
void UPlayerUI::ShowPickupMessage(const FString& Message)
{
    if (UTextBlock* Label = Cast<UTextBlock>(GetWidgetFromName(TEXT("PickupLabel"))))
    {
        if (Message == TEXT(""))
        {
            Label->SetVisibility(ESlateVisibility::Hidden);
            return;
		}
        Label->SetText(FText::Format(
            FText::FromString(TEXT("{0}\n(Press F)")),
            FText::FromString(Message)
        ));
        Label->SetVisibility(ESlateVisibility::Visible);
    }
}

void UPlayerUI::HidePickupMessage()
{
    if (UTextBlock* Label = Cast<UTextBlock>(GetWidgetFromName(TEXT("PickupLabel"))))
    {
        Label->SetVisibility(ESlateVisibility::Hidden);
    }
}

void UPlayerUI::UpdateHealth(float CurrentHealth, float MaxHealth)
{
    if (HpBar && MaxHealth > 0.0f)
    {
        HpBar->SetPercent(CurrentHealth / MaxHealth);
    }
	HpValueLb->SetText(FText::AsNumber(FMath::RoundToInt(CurrentHealth)));
}

void UPlayerUI::UpdateAmmo(int CurrentAmmoValue, int RemainAmmoValue)
{
    if (CurrentAmmo)
    {
        CurrentAmmo->SetText(FText::AsNumber(CurrentAmmoValue));
    }
    if (RemainingAmmo)
    {
        RemainingAmmo->SetText(FText::AsNumber(RemainAmmoValue));
    }
}

void UPlayerUI::UpdateTeamScores(int FirstScore, int SecondScore)
{
    if (FirstTeamScore)
    {
        FirstTeamScore->SetText(FText::AsNumber(FirstScore));
    }
    if (SecondTeamScore)
    {
        SecondTeamScore->SetText(FText::AsNumber(SecondScore));
    }
}

void UPlayerUI::OnHit()
{
    if (GetHitAnim)
    {
        PlayAnimation(GetHitAnim);
    }
}

void UPlayerUI::OnEnter()
{
    // This function can be used to initialize or reset UI elements when the player enters the game
    ShowPickupMessage(TEXT(""));

    KillNotifyStack->ClearChildren();

	ShowIconGrenade(EItemId::GRENADE_FRAG_BASIC, true);
	ShowIconGrenade(EItemId::GRENADE_SMOKE, false);
	ShowIconGrenade(EItemId::GRENADE_STUN, false);
	ShowIconGrenade(EItemId::GRENADE_INCENDIARY, false);
	ScopeUI->HideScope();
	PnSpike->SetVisibility(ESlateVisibility::Hidden);
	NotiToastPn->SetVisibility(ESlateVisibility::Hidden);
	SwitchSideEffPn->SetVisibility(ESlateVisibility::Hidden);   
    if (MatchToastPn) {
        MatchToastPn->SetVisibility(ESlateVisibility::Hidden);
    }
}

void UPlayerUI::NotifyKill(const AMyPlayerState* Killer, const AMyPlayerState* Victim, const UItemConfig* WeaponConf, bool bIsHeadShot)
{
    if (KillNotifyWidgetClass && KillNotifyStack)
    {
        UKillNotifySlot* KillNotifyWidget = CreateWidget<UKillNotifySlot>(GetWorld(), KillNotifyWidgetClass);
        if (KillNotifyWidget)
        {
            KillNotifyWidget->SetInfo(Killer, Victim, WeaponConf, bIsHeadShot);
            KillNotifyStack->AddChild(KillNotifyWidget);
            FTimerHandle TimerHandle;
            TWeakObjectPtr<UKillNotifySlot> WeakKillNotifyWidget = KillNotifyWidget;
            GetWorld()->GetTimerManager().SetTimer(
                TimerHandle,
                FTimerDelegate::CreateLambda([WeakKillNotifyWidget]()
                    {
                        if (WeakKillNotifyWidget.IsValid())
                        {
                            WeakKillNotifyWidget->RemoveFromParent();
                        }
                    }),
                5.0f,
                false
            );
        }
    }
}

void UPlayerUI::OpenShop()
{
    if (!WBP_Shop) {
		UE_LOG(LogTemp, Warning, TEXT("OpenShop: WBP_Shop is null"));
        return;
    }

    WBP_Shop->SetVisibility(ESlateVisibility::Visible);
	WBP_Shop->OnActive();
}

void UPlayerUI::CloseShop()
{
	UE_LOG(LogTemp, Warning, TEXT("CloseShop: Closing shop UI"));
    if (!WBP_Shop) {
		UE_LOG(LogTemp, Warning, TEXT("CloseShop: WBP_Shop is null"));
        return;
    }

    WBP_Shop->SetVisibility(ESlateVisibility::Hidden);
}

void UPlayerUI::ShowIconGrenade(EItemId ItemId, bool bShow)
{
   
}


void UPlayerUI::CreateGrenadeNodes()
{
    if (!GrenadeNodeClass || !GrenadesStack)
        return;

    Grenades.Empty();
    GrenadesStack->ClearChildren();

    for (int i = 0; i < 4; i++)
    {
        UGrenadeNodeUI* Node = CreateWidget<UGrenadeNodeUI>(GetWorld(), GrenadeNodeClass);
        if (!Node)
        {
            continue;
        }

        GrenadesStack->AddChild(Node);
        Grenades.Add(Node);
    }
}

void UPlayerUI::UpdateGrenades(const TArray<EItemId>& GrenadeIds)
{
    // 1. Reset all nodes
    for (UGrenadeNodeUI* Grenade : Grenades)
    {
        if (Grenade)
        {
            Grenade->UpdateIcon(EItemId::NONE);
        }
    }

    // 2. Update existing ones
    const int32 Count = FMath::Min(GrenadeIds.Num(), Grenades.Num());

    for (int32 i = 0; i < Count; i++)
    {
        if (Grenades[i])
        {
            Grenades[i]->UpdateIcon(GrenadeIds[i]);
        }
    }
}

void UPlayerUI::UpdateSpikeSlot(bool bHasSpike)
{
    if (SpikeSlot)
    {
        SpikeSlot->SetVisibility(bHasSpike ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
    }
}

void UPlayerUI::UpdatePistol(const EItemId& ItemId) {
    if (ItemId == EItemId::NONE) {
        Pistol->SetVisibility(ESlateVisibility::Collapsed);
    } else {
        Pistol->SetVisibility(ESlateVisibility::Visible);
        
		const UItemConfig* WeaponConf = UItemsManager::Get(GetWorld())->GetItemById(ItemId);
        if (WeaponConf == nullptr) {
            return;
        }
        PistolIcon->SetBrushFromTexture(WeaponConf->ItemIcon.Get());
    }
}

void UPlayerUI::UpdateRifle(const EItemId& ItemId) {
    if (ItemId == EItemId::NONE) {
        Rifle->SetVisibility(ESlateVisibility::Collapsed);
    }
    else {
        Rifle->SetVisibility(ESlateVisibility::Visible);
        
        const UItemConfig* WeaponConf = UItemsManager::Get(GetWorld())->GetItemById(ItemId);
        if (WeaponConf == nullptr) {
            return;
        }
        RifleIcon->SetBrushFromTexture(WeaponConf->ItemIcon.Get());
    }
}

void UPlayerUI::UpdateCurrentWeapon(EItemId CurrentWeaponId) {
    GrenadeTitle->SetText(FText::GetEmpty());
    WBP_Crosshair->SetVisibility(ESlateVisibility::Visible);

    for (UGrenadeNodeUI* Grenade : Grenades)
    {
        if (Grenade)
        {
            Grenade->SetSelected(false);
        }
	}
    if (CurrentWeaponId == EItemId::NONE) {

    }
    else {
		const UItemConfig* ItemConf = UItemsManager::Get(GetWorld())->GetItemById(CurrentWeaponId);
        if (ItemConf == nullptr) {
            return;
        }
		AmmoPn->SetVisibility(ItemConf->GetItemType() == EItemType::Firearm ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
        if (ItemConf->GetItemType() == EItemType::Throwable) {
            GrenadeTitle->SetText(ItemConf->DisplayName);
            for (UGrenadeNodeUI* Grenade : Grenades)
            {
                if (Grenade && Grenade->CurItemId == CurrentWeaponId)
                {
                    Grenade->SetSelected(true);
                    break;
                }
			}
        }
        else if (ItemConf->GetItemType() == EItemType::Firearm) {
			const UFirearmConfig* FirearmConf = Cast<UFirearmConfig>(ItemConf);
            if (FirearmConf->FirearmType == EFirearmType::Rifle) {
                //Rifle
                if (RifleIcon) {
                    // set texture
                    if (FirearmConf->ItemIcon) {
                        RifleIcon->SetBrushFromTexture(FirearmConf->ItemIcon.Get());
                    }
				}
            }
            else {
                //Pistol
                if (PistolIcon) {
                    // set texture
                    if (FirearmConf->ItemIcon) {
                        PistolIcon->SetBrushFromTexture(FirearmConf->ItemIcon.Get());
                    }
                }
            }
           
            if (FirearmConf->bHasScopeEquipped) {
                WBP_Crosshair->SetVisibility(ESlateVisibility::Hidden);
            }
        }

        if (CurrentItemIcon) {
            // set texture
            if (ItemConf->ItemIcon) {
                CurrentItemIcon->SetBrushFromTexture(ItemConf->ItemIcon.Get());
            }
        }
    }
	ShowWeaponGuide();
}

void UPlayerUI::ShowWeaponGuide()
{
	PlayAnimation(ShowWeaponNumbers);
	PlayAnimation(ShowWeaponIcons);
}


void UPlayerUI::ShowScope()
{
    if (UGameManager* GM = UGameManager::Get(GetWorld()))
    {
        if (GM->GlobalData && GM->GlobalData->ZoomScopeSound)
        {
            UGameplayStatics::PlaySound2D(GetWorld(), GM->GlobalData->ZoomScopeSound.Get());
        }
    }
    ScopeUI->ShowScope();
}

void UPlayerUI::HideScope()
{
    ScopeUI->HideScope();
}

void UPlayerUI::OnUpdatePlantSpikeState(bool IsPlanting) {
    if (IsPlanting) {
		PnSpike->SetVisibility(ESlateVisibility::Visible);
        PlayAnimation(StartPlantSpikeAnim);
    } else {
        StopAnimation(StartPlantSpikeAnim);
		PnSpike->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UPlayerUI::OnUpdateDefuseSpikeState(bool IsDefusing) {
	UE_LOG(LogTemp, Warning, TEXT("OnUpdateDefuseSpikeState: IsDefusing = %s"), IsDefusing ? TEXT("true") : TEXT("false"));
    if (IsDefusing) {
        PnSpike->SetVisibility(ESlateVisibility::Visible);
        PlayAnimation(StartDefuseSpikeAnim);
    }
    else {
        StopAnimation(StartDefuseSpikeAnim);
        PnSpike->SetVisibility(ESlateVisibility::Hidden);
    }
}

void UPlayerUI::ShowMatchStateToast(FText Txt, float Delay)
{
    if (Delay <= 0.f)
    {
        // run immediately
        DoShowMatchStateToast(Txt);
        return;
    }

    // Capture text for delayed execution
    FText LocalText = Txt;

    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(
        TimerHandle,
        [this, LocalText]()
        {
            DoShowMatchStateToast(LocalText);
        },
        Delay,
        false
    );
}
void UPlayerUI::DoShowMatchStateToast(FText Txt)
{
    if (MatchToastLb)
    {
        MatchToastLb->SetText(Txt);
        MatchToastPn->SetVisibility(ESlateVisibility::Visible);
        PlayAnimation(ShowMatchStateAnim);
    }
}

void UPlayerUI::ShowNotiToast(FText Txt)
{
    if (NotiToastLb && NotiToastPn)
    {
        NotiToastLb->SetText(Txt);
        NotiToastPn->SetVisibility(ESlateVisibility::Visible);
        PlayAnimation(ShowNotiToastAnim);
    }
}

void UPlayerUI::OnUpdateRoundTime(int TimeEnd) {
	RoundTimeEnd = TimeEnd;
	StartRoundClock();
}

void UPlayerUI::UpdateGameState(const EMyMatchState& State) {
	PhotonPlantedIcon->SetVisibility(ESlateVisibility::Hidden);
	MatchTimeLb->SetVisibility(ESlateVisibility::Visible);
    bPlayedTenSec = false;
	MatchStatePn->SetVisibility(ESlateVisibility::Hidden);
    switch (State) {
        case EMyMatchState::PRE_MATCH:
			MatchStatePn->SetVisibility(ESlateVisibility::Visible);
			MatchStateTxt->SetText(FText::FromString("Waiting for other players …"));
			break;
        case EMyMatchState::ROUND_START:
			ShowMatchStateToast(FText::FromString("Round Started!"), 0.f);
            break;
        case EMyMatchState::BUY_PHASE:
			/*MatchStatePn->SetVisibility(ESlateVisibility::Visible);
			MatchStateTxt->SetText(FText::FromString("Buy Phase (Press B to Open Shop)"));*/
			break;
        case EMyMatchState::ROUND_IN_PROGRESS:
			//ShowMatchStateToast(FText::FromString("Round In Progress"), 0.f);
            break;
		case EMyMatchState::SPIKE_PLANTED:
			MatchTimeLb->SetVisibility(ESlateVisibility::Hidden);
			PhotonPlantedIcon->SetVisibility(ESlateVisibility::Visible);
			PlayAnimation(PhotonPlantedAnim, 0.f, 0);
            ShowMatchStateToast(FText::FromString("Spike Planted!"), 0.f);
            break;
		case EMyMatchState::ROUND_ENDED:
            //ShowMatchStateToast(FText::FromString("Round Ended"), 0.f);
			break;
    }
}

void UPlayerUI::ShowScoreboard(bool bShow) {
    if (UWidget* ScoreboardPn = GetWidgetFromName(TEXT("ScoreboardPn"))) {
        ScoreboardPn->SetVisibility(bShow ? ESlateVisibility::Visible : ESlateVisibility::Hidden);

		UScoreboardUI* ScoreboardWidget = Cast<UScoreboardUI>(ScoreboardPn);
        if (ScoreboardWidget) {
            if (bShow) {
                AShooterGameState* GS = GetWorld()->GetGameState<AShooterGameState>();
                ScoreboardWidget->UpdateScoreboard(GS);
            }
        }
    }
}

void UPlayerUI::UpdateArmor(int ArmorPoints, int MaxArmorPoints) {
	float ArmorPercent = MaxArmorPoints > 0 ? static_cast<float>(ArmorPoints) / static_cast<float>(MaxArmorPoints) : 0.0f;
    if (ArmorBar) {
        ArmorBar->SetPercent(ArmorPercent);
	}
    if (AmmorPointLb) {
        AmmorPointLb->SetText(FText::AsNumber(ArmorPoints));
    }
    if (ArmorPoints <= 0) {
       
    }
    else {
 
    }
}

void UPlayerUI::ShowKillMark(bool bHeadShot) {
    if (KillMarkIcon) {
        KillMarkIcon->SetVisibility(ESlateVisibility::Visible);
        PlayAnimation(KillMarkAnim);

        // update icons
		UGameManager* GM = UGameManager::Get(GetWorld());
		UGlobalDataAsset* GlobalData = GM->GlobalData;
        
        if (bHeadShot) {
            if (GlobalData->KillMarkHeadshotIcon) {
                KillMarkIcon->SetBrushFromTexture(GlobalData->KillMarkHeadshotIcon.Get());
            }

            if (GlobalData->KillHeadshotSound) {
                UGameplayStatics::PlaySound2D(GetWorld(), GlobalData->KillHeadshotSound.Get());
            }
        }
        else {
            if (GlobalData->KillMarkNormalIcon) {
                KillMarkIcon->SetBrushFromTexture(GlobalData->KillMarkNormalIcon.Get());
            }

            if (GlobalData->KillMarkSound) {
				UGameplayStatics::PlaySound2D(GetWorld(), GlobalData->KillMarkSound.Get());
            }
		}
    }
}

void UPlayerUI::StartRoundClock()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
	UE_LOG(LogTemp, Log, TEXT("UPlayerUI::StartRoundClock: Starting round clock with end time %d"), RoundTimeEnd);

    // Run immediately and then every 1s
    UpdateRoundClockOnce();

    World->GetTimerManager().ClearTimer(RoundClockTimerHandle);
    World->GetTimerManager().SetTimer(
        RoundClockTimerHandle,
        this,
        &UPlayerUI::UpdateRoundClockOnce,
        1.0f,
        true
    );
}

void UPlayerUI::StopRoundClock()
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(RoundClockTimerHandle);
    }

    if (MatchTimeLb) {
        MatchTimeLb->SetText(FText::FromString(TEXT("")));
	}
}

void UPlayerUI::UpdateRoundClockOnce()
{
    if (RoundTimeEnd <= 0 || !MatchTimeLb)
    {
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    AShooterGameState* GS = World->GetGameState<AShooterGameState>();
    if (!GS)
    {
        return;
    }

    const int32 Now = FMath::CeilToInt(World->GetTimeSeconds());
    const int32 Remaining = FMath::Max(RoundTimeEnd - Now, 0);

    // 10-second warning (same logic as before)
    if (!bPlayedTenSec
        && Remaining == 10
        && GS->GetMatchMode() == EMatchMode::Zombie
        && GS->GetMatchState() == EMyMatchState::BUY_PHASE)
    {
        bPlayedTenSec = true;

        if (UGameManager* GM = UGameManager::Get(World))
        {
            if (GM->GlobalData && GM->GlobalData->CountdownTenSound)
            {
                UGameplayStatics::PlaySound2D(World, GM->GlobalData->CountdownTenSound.Get());
            }
        }
    }

    const int32 Minutes = Remaining / 60;
    const int32 Seconds = Remaining % 60;

    MatchTimeLb->SetText(FText::FromString(FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds)));

    const FLinearColor NormalColor = FLinearColor::FromSRGBColor(FColor::FromHex(TEXT("FFFACDFF")));
    MatchTimeLb->SetColorAndOpacity((Remaining <= 10) ? FLinearColor::Red : NormalColor);

    if (Remaining == 0)
    {
        StopRoundClock();
    }
}

void UPlayerUI::UpdatePlayerName(const FString& PlayerName) {
    if (PlayerNameLb) {
        PlayerNameLb->SetText(FText::FromString(PlayerName));
    }
}

void UPlayerUI::SetRadarVisible(bool bVisible)
{
    if (!RadarWidget) return;

    RadarWidget->SetVisibility(
        bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed
    );
}

void UPlayerUI::SetMatchInfoPnVisible(bool bVisible)
{
    if (MatchInfoPn)
    {
        MatchInfoPn->SetVisibility(
            bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed
        );
    }
}

void UPlayerUI::ShowGameResult(ETeamId WinTeam) {
    UGameManager* GM = UGameManager::Get(GetWorld());
    UGlobalDataAsset* GlobalData = GM->GlobalData;
    UGameplayStatics::PlaySound2D(
        GetWorld(),
        GM->GlobalData->GameEndSound    
    );

    ShowMatchStateToast(
        FText::FromString(
            WinTeam == ETeamId::Attacker ? "Attackers Win!" :
            WinTeam == ETeamId::Defender ? "Defenders Win!" :
            "Game Ended!"
        ),
        0.f
	);
	UE_LOG(LogTemp, Log, TEXT("UPlayerUI::ShowGameResult: WinTeam = %d"), static_cast<int32>(WinTeam));
}

void UPlayerUI::OnSwitchSide() {
    UGameManager* GM = UGameManager::Get(GetWorld());
    UGlobalDataAsset* GlobalData = GM->GlobalData;
    UGameplayStatics::PlaySound2D(
        GetWorld(),
        GM->GlobalData->SwitchingSideVoice
    );

    FTimerHandle ShowTimerHandle;
	SwitchSideEffPn->SetVisibility(ESlateVisibility::Visible);

    GetWorld()->GetTimerManager().SetTimer(
        ShowTimerHandle,
        [this]()
        {
            PlayAnimation(ShowAnimSwitchSide);
        },
        1.0f,
        false
    );

	// hide after 2 seconds
    FTimerHandle HideTimerHandle;
    GetWorld()->GetTimerManager().SetTimer(
        HideTimerHandle,
        [this]()
        {
            SwitchSideEffPn->SetVisibility(ESlateVisibility::Hidden);
        },
        5.0f,
        false
    );
}

void UPlayerUI::UpdateTeamId(ETeamId TeamId) {
    UGameManager* GM = UGameManager::Get(GetWorld());
    UGlobalDataAsset* GlobalData = GM->GlobalData;

    if (TeamId == ETeamId::Attacker) {
        TeamIcon->SetBrushFromTexture(GlobalData->AttackerIcon.Get());
    }
    else if (TeamId == ETeamId::Defender) {
        TeamIcon->SetBrushFromTexture(GlobalData->DefenderIcon.Get());
    }
    else {
        TeamIcon->SetVisibility(ESlateVisibility::Hidden);
    }
}

void UPlayerUI::UpdateRoundNumber() {
	AShooterGameState* GS = GetWorld() ? GetWorld()->GetGameState<AShooterGameState>() : nullptr;
    if (!GS) {
        return;
	}
    RoundLb->SetText(
        FText::Format(
            FText::FromString("Round {0}/{1}"),
            FText::AsNumber(GS->GetCurrentRound()),
            FText::AsNumber(FGameConstants::MAX_ROUND_ZOMBIE_MODE)
        )
    );

}

void UPlayerUI::UpdateHeroPhase() {
    AShooterGameState* GS = GetWorld() ? GetWorld()->GetGameState<AShooterGameState>() : nullptr;
    if (!GS) {
        return;
    }
    if (GS->IsHeroPhase()) {
        // play sound
        UGameManager* GM = UGameManager::Get(GetWorld());
        UGlobalDataAsset* GlobalData = GM->GlobalData;
        UGameplayStatics::PlaySound2D(
            GetWorld(),
            GM->GlobalData->HeroPhaseActiveSound
        );
		ShowNotiToast(FText::FromString("Press E to become hero!"));
		ZombieVsHeroPn->SetVisibility(ESlateVisibility::Visible);
    }
    else {
		ZombieVsHeroPn->SetVisibility(ESlateVisibility::Hidden);
	}
}