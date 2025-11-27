// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/PlayerUI.h"
#include "Game/TeamEliminationState.h"

void UPlayerUI::NativeConstruct()
{
    Super::NativeConstruct();
    if (HpBar)
    {
        HpBar->SetPercent(1.0f);
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
    ATeamEliminationState* GST = GetWorld()->GetGameState<ATeamEliminationState>();
    if (GST) {
        // Assuming we have a way to determine which team is "my team"
        int MyTeamPoints = GST->TeamAScore; // Replace with actual team logic
        int OpponentTeamPoints = GST->TeamBScore; // Replace with actual team logic
		UpdateTeamScores(MyTeamPoints, OpponentTeamPoints);
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
	FlashScreen->SetVisibility(ESlateVisibility::Hidden);

	ShowIconGrenade(EItemId::GRENADE_FRAG_BASIC, true);
	ShowIconGrenade(EItemId::GRENADE_SMOKE, false);
	ShowIconGrenade(EItemId::GRENADE_STUN, false);
	ShowIconGrenade(EItemId::GRENADE_INCENDIARY, false);
}

void UPlayerUI::NotifyKill(const FString& KillerName, const FString& VictimName, UTexture2D* WeaponTex, bool bIsHeadShot)
{
    if (KillNotifyWidgetClass && KillNotifyStack)
    {
        UKillNotifySlot* KillNotifyWidget = CreateWidget<UKillNotifySlot>(GetWorld(), KillNotifyWidgetClass);
        if (KillNotifyWidget)
        {
            KillNotifyWidget->SetInfo(KillerName, VictimName, WeaponTex, bIsHeadShot);
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

    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC) {
		UE_LOG(LogTemp, Warning, TEXT("OpenShop: PlayerController is null"));
        return;
    }

    PC->bShowMouseCursor = true;

    //FInputModeUIOnly InputMode;
    //InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    //InputMode.SetWidgetToFocus(TakeWidget()); // focus this widget
    //PC->SetInputMode(InputMode);

    // Optionally disable pawn input
    if (APawn* Pawn = PC->GetPawn())
    {
        Pawn->DisableInput(PC);
    }
}

void UPlayerUI::CloseShop()
{
	UE_LOG(LogTemp, Warning, TEXT("CloseShop: Closing shop UI"));
    if (!WBP_Shop) {
		UE_LOG(LogTemp, Warning, TEXT("CloseShop: WBP_Shop is null"));
        return;
    }

    WBP_Shop->SetVisibility(ESlateVisibility::Hidden);

    APlayerController* PC = GetWorld()->GetFirstPlayerController();
    if (!PC) {
		UE_LOG(LogTemp, Warning, TEXT("CloseShop: PlayerController is null"));
        return;
    }
	UE_LOG(LogTemp, Warning, TEXT("CloseShop: Hiding mouse cursor"));


    FInputModeGameOnly GameInput;
    PC->SetInputMode(GameInput);
    PC->bShowMouseCursor = false;

    if (APawn* Pawn = PC->GetPawn())
    {
        Pawn->EnableInput(PC);
    }
}

void UPlayerUI::ShowIconGrenade(EItemId ItemId, bool bShow)
{
    if (ItemId == EItemId::GRENADE_FRAG_BASIC)
    {
        if (IconGrenadeFrag)
        {
            IconGrenadeFrag->SetVisibility(bShow ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
        }
        if (DotFrag)
        {
            DotFrag->SetVisibility(!bShow ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
		}
    }
    else if (ItemId == EItemId::GRENADE_SMOKE)
    {
        if (IconGrenadeSmoke)
        {
            IconGrenadeSmoke->SetVisibility(bShow ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
        }
        if (DotSmoke)
        {
            DotSmoke->SetVisibility(!bShow ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
        }
    }
    else if (ItemId == EItemId::GRENADE_STUN)
    {
        if (IconGrenadeFlash)
        {
            IconGrenadeFlash->SetVisibility(bShow ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
        }
        if (DotFlash)
        {
            DotFlash->SetVisibility(!bShow ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
		}
    }
    else if (ItemId == EItemId::GRENADE_INCENDIARY)
    {
        if (IconGrenadeIncendiary)
        {
            IconGrenadeIncendiary->SetVisibility(bShow ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
        }
        if (DotIncen)
        {
            DotIncen->SetVisibility(!bShow ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
        }
	}
}
