// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/UI/PlayerSlotUI.h"
#include "Game/Framework/PlayerSlot.h"
#include "Shared/System/ContentRegistrySubsystem.h"
#include "Game/Characters/BaseCharacter.h"
#include "Game/Characters/Components/HealthComponent.h"
#include "Shared/Utils/GameUtils.h"
#include "Game/Framework/ShooterGameState.h"

void UPlayerSlotUI::NativeConstruct()
{
	Super::NativeConstruct();

	HpBar->SetPercent(.0f);
	DeadIcon->SetVisibility(ESlateVisibility::Collapsed);
	DisconnectIcon->SetVisibility(ESlateVisibility::Collapsed);
}

void UPlayerSlotUI::SetInfo(APlayerSlot* SlotInfo, bool bInIsMyTeam)
{
	bIsMyTeam = bInIsMyTeam;
	HpBar->SetVisibility(bIsMyTeam ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	CachedSlot = SlotInfo;
	if (SlotInfo)
	{
		auto PName = GameUtils::SubStringWithDots(SlotInfo->GetPlayerName(), 8);
		PlayerNameLb->SetText(FText::FromString(PName));

		FSlateBrush Brush = AvatarImg->GetBrush();
		if (SlotInfo->GetTeamId() == ETeamId::Attacker)
		{
			// Dark red
			Brush.OutlineSettings.Color = FLinearColor(0.768f, 0.156f, 0.0021f, 1.0f);
		}
		else
		{
			// Dark blue
			Brush.OutlineSettings.Color = FLinearColor(0.1f, 0.2f, 0.6f, 1.0f);
		}
		AvatarImg->SetBrush(Brush);

		UContentRegistrySubsystem* Registry =
			GetGameInstance()->GetSubsystem<UContentRegistrySubsystem>();
		AvatarImg->SetBrushFromTexture(
			Registry->GetAvatarTextureById(SlotInfo->GetAvatar()));

		SlotInfo->OnReplicatedPawnChanged.AddUObject(this, &UPlayerSlotUI::HandlePawnChanged);
		SlotInfo->OnUpdateConnectedStatus.AddUObject(this, &UPlayerSlotUI::HandleUpdateConnectedStatus);
		HandleUpdateConnectedStatus(SlotInfo->IsConnected());
		HandlePawnChanged();
	}
}

void UPlayerSlotUI::HandleHealthChanged(float NewHealth, float MaxHealth)
{
	if (MaxHealth <= 0.f) {
		return;
	}
	if (NewHealth <= 0.f) {
		this->SetRenderOpacity(0.5f);
		DeadIcon->SetVisibility(ESlateVisibility::Visible);
	}
	else {
		this->SetRenderOpacity(1.0f);
		DeadIcon->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (bIsMyTeam) {
		// Only show health for my team members
		float HealthPercent = NewHealth / MaxHealth;
		HpBar->SetPercent(HealthPercent);
	}
}

void UPlayerSlotUI::HandlePawnChanged()
{
	HpBar->SetPercent(.0f);

	TriedNum = 0;
	StartRetryBindPawn();
}

void UPlayerSlotUI::StartRetryBindPawn()
{
	// Clear old timer if any
	GetWorld()->GetTimerManager().ClearTimer(BindPawnTimer);

	// Try immediately
	if (TryToBindPawn())
	{
		return;
	}

	// Retry each 1s
	GetWorld()->GetTimerManager().SetTimer(
		BindPawnTimer,
		this,
		&UPlayerSlotUI::RetryBindPawnTick,
		1.0f,
		true
	);
}

void UPlayerSlotUI::RetryBindPawnTick()
{
	++TriedNum;

	if (TryToBindPawn() || TriedNum >= MaxTries)
	{
		GetWorld()->GetTimerManager().ClearTimer(BindPawnTimer);
	}
}


bool UPlayerSlotUI::TryToBindPawn()
{
	if (!CachedSlot) return false;

	ABaseCharacter* Char = Cast<ABaseCharacter>(CachedSlot->GetPawn());
	if (!IsValid(Char)) {
		return false;
	}

	UHealthComponent* HealthComp = Char->GetHealthComponent();
	if (!IsValid(HealthComp)) {
		return false;
	}

	HealthComp->OnHealthUpdated.RemoveAll(this);

	HealthComp->OnHealthUpdated.AddUObject(this, &UPlayerSlotUI::HandleHealthChanged);
	// Initial update
	HandleHealthChanged(HealthComp->GetHealth(), HealthComp->GetMaxHealth());
	return true;
}

void UPlayerSlotUI::HandleUpdateConnectedStatus(bool bIsConnected)
{
	if (bIsConnected)
	{
		if (!CachedSlot) {
			this->SetRenderOpacity(1.0f);
			return;
		}
		ABaseCharacter* Char = Cast<ABaseCharacter>(CachedSlot->GetPawn());
		if (IsValid(Char)) {
			if (Char->IsPermanentDead()) {
				this->SetRenderOpacity(0.6f);
				return;
			}
		}

		this->SetRenderOpacity(1.0f);
		DisconnectIcon->SetVisibility(ESlateVisibility::Collapsed);
	}
	else
	{
		this->SetRenderOpacity(0.4f);

		AShooterGameState* GS = GetWorld()->GetGameState<AShooterGameState>();
		if (GS && GS->GetMatchState() != EMyMatchState::PRE_MATCH) {
			DisconnectIcon->SetVisibility(ESlateVisibility::Visible);
		}
	}
}
