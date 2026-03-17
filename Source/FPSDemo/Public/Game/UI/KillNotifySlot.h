// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/StackBox.h"
#include "Game/Framework/MyPlayerState.h"
#include "KillNotifySlot.generated.h"

class UItemConfig;
/**
 * 
 */
UCLASS()
class FPSDEMO_API UKillNotifySlot : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* KillerLb;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* VictimLb;
	UPROPERTY(meta = (BindWidget))
	UImage* WeaponIcon;
	UPROPERTY(meta = (BindWidget))
	UImage* HeadShotIcon;
	UPROPERTY(meta = (BindWidget))
	UImage* PenetrationIcon;

	void SetInfo(const AMyPlayerState* Killer, const AMyPlayerState* Victem, 
		const UItemConfig* WeaponTex, bool bIsHeadShot, bool bIsPenetrationHit);
};
