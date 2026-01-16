// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "Lobby/RoomData.h"
#include "RoomPlayerSlotUI.generated.h"


DECLARE_MULTICAST_DELEGATE_OneParam(FOnDeletePlayer, int);
/**
 * 
 */
UCLASS()
class FPSDEMO_API URoomPlayerSlotUI : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	UImage* AvatarImg;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* NameLb;

	UPROPERTY(meta = (BindWidget))
	UWidget* EmptyPn;

	UPROPERTY(meta = (BindWidget))
	UButton* SwitchBtn;

	UPROPERTY(meta = (BindWidget))
	UButton* DeleteBtn;	
public:
	void SetPlayerInfo(PlayerRoomInfo Info, int InSlotIdx);
	bool IsEmpty() const;
	PlayerRoomInfo GetPlayerInfo() const { return  CachedPlayerInfo; }

	FOnDeletePlayer OnDeletePlayer;
private:
	PlayerRoomInfo CachedPlayerInfo;
	int SlotIdx;

	UFUNCTION()
	void OnSwitchClicked();
	UFUNCTION()
	void OnDeleteClicked();
};
