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
