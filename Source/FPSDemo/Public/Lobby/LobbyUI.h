// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "Lobby/RoomData.h"
#include "Lobby/RoomPlayerSlotUI.h"
#include "LobbyUI.generated.h"

namespace LobbyUIColor
{
	static const FLinearColor Selected = FLinearColor(0.7216f, 0.7333f, 0.4431f, 1.0f); // #40488DFF
	static const FLinearColor Unselected = FLinearColor(0.733f, 0.733f, 0.733f, 1.0f); // #BBBBBBFF
}

class UNetworkManager;
/**
 * 
 */
UCLASS()
class FPSDEMO_API ULobbyUI : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	UButton* CreateRoomBtn;

	UPROPERTY(meta = (BindWidget))
	UWidget* CreatePn;

	UPROPERTY(meta = (BindWidget))
	UWidget* ListPn;

	UPROPERTY(meta = (BindWidget))
	UWidget* VsTxt;

	UPROPERTY(meta = (BindWidget))
	UButton* AddBotTeam1Btn;

	UPROPERTY(meta = (BindWidget))
	UButton* AddBotTeam2Btn;

	UPROPERTY(meta = (BindWidget))
	UButton* ZombieModeBtn;

	UPROPERTY(meta = (BindWidget))
	UButton* BombModeBtn;

	UPROPERTY(meta = (BindWidget))
	UButton* BtnSelfHost;

	UPROPERTY(meta = (BindWidget))
	UButton* BtnDedicatedServer;

	UPROPERTY(meta = (BindWidget))
	UButton* StartBtn;

	UPROPERTY(meta = (BindWidget))
	UWidget* BombModeTickIcon;

	UPROPERTY(meta = (BindWidget))
	UWidget* ZombieTickIcon;

	UPROPERTY(meta = (BindWidget))
	UWidget* SelfHostTickIcon;

	UPROPERTY(meta = (BindWidget))
	UWidget* DedicatedTickIcon;
	
	UPROPERTY();
	TArray<URoomPlayerSlotUI*> PlayerSlotUIs;
private:
	UFUNCTION()
	void OnZombieModeClicked();
	UFUNCTION()
	void OnBombModeClicked();
	UFUNCTION()
	void OnSelfHostClicked();
	UFUNCTION()
	void OnDedicatedServerClicked();
	void SetMatchMode(EMatchMode InMode);
	void SetButtonNormalColor(UButton* Button, const FLinearColor& Color);
	void SetHostMode(bool bIsSelfHost);
	UFUNCTION()
	void OnStartGameClicked();
	UFUNCTION()
	void OnAddBotTeam1();
	UFUNCTION()
	void OnAddBotTeam2();

	UFUNCTION()
	void OnCreateRoomClicked();

	void OnDeletePlayer(int32 PlayerId);
	void UpdateRoomData();
	void HandleCreateRoomSuccess();

	FRoomData CurrentRoomData;
	UNetworkManager* CachedNetworkManager;
};
