// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PlayerUI.h"
#include "Game/TeamEliminationState.h"
#include "Game/GameManager.h"

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
}
void UPlayerUI::ShowPickupMessage(const FString& Message)
{
    if (UTextBlock* Label = Cast<UTextBlock>(GetWidgetFromName(TEXT("PickupLabel"))))
    {
        Label->SetText(FText::FromString(Message));
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

void UPlayerUI::UpdateAmmo(int CurrentAmmoValue, int TotalAmmoValue)
{
    if (CurrentAmmo)
    {
        CurrentAmmo->SetText(FText::AsNumber(CurrentAmmoValue));
    }
    if (TotalAmmo)
    {
        TotalAmmo->SetText(FText::AsNumber(TotalAmmoValue));
    }
}

void UPlayerUI::UpdateTeamScores(int MyTeamPoints, int OpponentTeamPoints)
{
    if (MyTeamScore)
    {
        MyTeamScore->SetText(FText::AsNumber(MyTeamPoints));
    }
    if (OpponentTeamScore)
    {
        OpponentTeamScore->SetText(FText::AsNumber(OpponentTeamPoints));
    }
}

void UPlayerUI::OnUpdateScore()
{
    // Get game state and update scores
  //  ATeamEliminationState* GST = GetWorld()->GetGameState<ATeamEliminationState>();
  //  if (GST) {
  //      // Assuming we have a way to determine which team is "my team"
  //      int MyTeamPoints = GST->TeamAScore; // Replace with actual team logic
  //      int OpponentTeamPoints = GST->TeamBScore; // Replace with actual team logic
		//UpdateTeamScores(MyTeamPoints, OpponentTeamPoints);
  //  }
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
	FlashScreen->SetVisibility(ESlateVisibility::Hidden);

	ShowIconGrenade(EItemId::GRENADE_FRAG_BASIC, true);
	ShowIconGrenade(EItemId::GRENADE_SMOKE, false);
	ShowIconGrenade(EItemId::GRENADE_STUN, false);
	ShowIconGrenade(EItemId::GRENADE_INCENDIARY, false);
	ScopeUI->HideScope();
	PnSpike->SetVisibility(ESlateVisibility::Hidden);
	NotiToastPn->SetVisibility(ESlateVisibility::Hidden);
    if (MatchToastPn) {
        MatchToastPn->SetVisibility(ESlateVisibility::Hidden);
    }
}

void UPlayerUI::NotifyKill(const FString& KillerName, const FString& VictimName, UWeaponData* WeaponConf, bool bIsHeadShot)
{
    if (KillNotifyWidgetClass && KillNotifyStack)
    {
        UKillNotifySlot* KillNotifyWidget = CreateWidget<UKillNotifySlot>(GetWorld(), KillNotifyWidgetClass);
        if (KillNotifyWidget)
        {
            KillNotifyWidget->SetInfo(KillerName, VictimName, WeaponConf, bIsHeadShot);
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

void UPlayerUI::ApplyFlashEffect(const float& Strength)
{
    FlashScreen->SetVisibility(ESlateVisibility::Visible);
    FLinearColor CurrentColor = FlashScreen->GetColorAndOpacity();
    CurrentColor.A = 1.0f;
    FlashScreen->SetColorAndOpacity(CurrentColor);
    UE_LOG(LogTemp, Warning, TEXT("Applying flash effect with strength: %f"), Strength);

    // Wait 3 seconds, then run FadeOutFlashEffect()
    FTimerHandle TimerHandle;
    GetWorld()->GetTimerManager().SetTimer(
        TimerHandle,
        this,
        &UPlayerUI::FadeOutFlashEffect,
        3.0f,
        false
    );
}

void UPlayerUI::FadeOutFlashEffect()
{
    if (FlashScreenAnim)
    {
        PlayAnimation(FlashScreenAnim);
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

void UPlayerUI::UpdatePistol(const EItemId& ItemId) {
    if (ItemId == EItemId::NONE) {
        Pistol->SetVisibility(ESlateVisibility::Collapsed);
    } else {
        Pistol->SetVisibility(ESlateVisibility::Visible);
        UGameManager* GMR = GetWorld()->GetGameInstance()->GetSubsystem<UGameManager>();
        UWeaponData* WeaponConf = GMR->GetWeaponDataById(ItemId);
        if (WeaponConf == nullptr) {
            return;
        }
        PistolIcon->SetBrushFromTexture(WeaponConf->Icon);
    }
}

void UPlayerUI::UpdateRifle(const EItemId& ItemId) {
    if (ItemId == EItemId::NONE) {
        Rifle->SetVisibility(ESlateVisibility::Collapsed);
    }
    else {
        Rifle->SetVisibility(ESlateVisibility::Visible);
        UGameManager* GMR = GetWorld()->GetGameInstance()->GetSubsystem<UGameManager>();
        UWeaponData* WeaponConf = GMR->GetWeaponDataById(ItemId);
        if (WeaponConf == nullptr) {
            return;
        }
        RifleIcon->SetBrushFromTexture(WeaponConf->Icon);
    }
}

void UPlayerUI::UpdateCurrentWeapon(const EItemId& CurrentWeaponId) {
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
        UGameManager* GMR = GetWorld()->GetGameInstance()->GetSubsystem<UGameManager>();
        UWeaponData* WeaponConf = GMR->GetWeaponDataById(CurrentWeaponId);
        if (WeaponConf == nullptr) {
            return;
        }
        if (WeaponConf->WeaponType == EWeaponTypes::Throwable) {
            GrenadeTitle->SetText(WeaponConf->DisplayName);
            for (UGrenadeNodeUI* Grenade : Grenades)
            {
                if (Grenade && Grenade->CurItemId == CurrentWeaponId)
                {
                    Grenade->SetSelected(true);
                    break;
                }
			}
        }
        else if (WeaponConf->WeaponType == EWeaponTypes::Firearm) {
            if (WeaponConf->WeaponSubType == EWeaponSubTypes::Rifle) {
                //Rifle
                if (RifleIcon) {
                    // set texture
                    RifleIcon->SetBrushFromTexture(WeaponConf->Icon);
				}
            }
            if (WeaponConf->HasScopeEquiped) {
                WBP_Crosshair->SetVisibility(ESlateVisibility::Hidden);
			}
        }
        else if (WeaponConf->WeaponSubType == EWeaponSubTypes::Pistol) {
            //Pistol
            if (PistolIcon) {
                // set texture
                PistolIcon->SetBrushFromTexture(WeaponConf->Icon);
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
    if (MatchStateLb)
    {
        MatchStateLb->SetText(Txt);
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