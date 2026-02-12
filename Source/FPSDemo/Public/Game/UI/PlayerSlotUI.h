// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/ProgressBar.h"
#include "PlayerSlotUI.generated.h"

class APlayerSlot;
/**
 * 
 */
UCLASS()
class FPSDEMO_API UPlayerSlotUI : public UUserWidget
{
	GENERATED_BODY()

private:
	const APlayerSlot* CachedSlot = nullptr;
	bool bIsMyTeam = false;

public:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* PlayerNameLb;
	UPROPERTY(meta = (BindWidget))
	UImage* AvatarImg;
	UPROPERTY(meta = (BindWidget))
	UImage* DeadIcon;
	UPROPERTY(meta = (BindWidget))
	UProgressBar* HpBar;

	void SetInfo(APlayerSlot* SlotInfo, bool bIsMyTeam);
	void HandlePawnChanged();

	UFUNCTION()
	void HandleHealthChanged(float NewHealth, float MaxHealth);

	FTimerHandle BindPawnTimer;
	int32 TriedNum = 0;
	static constexpr int32 MaxTries = 3;

	void StartRetryBindPawn();
	void RetryBindPawnTick();
	bool TryToBindPawn();

	void HandleUpdateConnectedStatus(bool bIsConnected);
	virtual void NativeConstruct() override;
};
