// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "Modules/Lobby/RoomPlayerSlotUI.h"
#include "game.pb.h"
#include "Components/VerticalBox.h"
#include "LobbyUI.generated.h"

namespace LobbyUIColor
{
	static const FLinearColor Selected = FLinearColor(0.7216f, 0.7333f, 0.4431f, 1.0f); // #40488DFF
	static const FLinearColor Unselected = FLinearColor(0.733f, 0.733f, 0.733f, 1.0f); // #BBBBBBFF
}

class UNetworkManager;
class URoomManager;
class URoomSlotUI;
class UChatUI;

/**
 * 
 */
UCLASS()
class FPSDEMO_API ULobbyUI : public UUserWidget
{
	GENERATED_BODY()

public:
	void ToggleChat();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* InsightDevLb;

	UPROPERTY(meta = (BindWidget))
	UButton* CreateRoomBtn;

	UPROPERTY(meta = (BindWidget))
	UWidget* RoomPn;

	UPROPERTY(meta = (BindWidget))
	UButton* PracticeBtn;

	UPROPERTY(meta = (BindWidget))
	UWidget* ListPn;

	UPROPERTY(meta = (BindWidget))
	UWidget* VsTxt;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* RoomTitleLb;

	UPROPERTY(meta = (BindWidget))
	UWidget* EmptyRoomTxt;

	UPROPERTY(meta = (BindWidget))
	UButton* AddBotTeam1Btn;

	UPROPERTY(meta = (BindWidget))
	UButton* AddBotTeam2Btn;

	UPROPERTY(meta = (BindWidget))
	UButton* ZombieModeBtn;

	UPROPERTY(meta = (BindWidget))
	UButton* BombModeBtn;

	UPROPERTY(meta = (BindWidget))
	UButton* DeathMatchModeBtn;

	UPROPERTY(meta = (BindWidget))
	UButton* SelfHostBtn;

	UPROPERTY(meta = (BindWidget))
	UButton* DedicatedServerBtn;

	UPROPERTY(meta = (BindWidget))
	UWidget* WaitingOwnerStartTxt;

	UPROPERTY(meta = (BindWidget))
	UButton* BackBtn;

	UPROPERTY(meta = (BindWidget))
	UButton* StartBtn;

	UPROPERTY(meta = (BindWidget))
	UWidget* BombModeTickIcon;

	UPROPERTY(meta = (BindWidget))
	UWidget* ZombieTickIcon;

	UPROPERTY(meta = (BindWidget))
	UWidget* SelfHostTickIcon;

	UPROPERTY(meta = (BindWidget))
	UWidget* DeathMatchTickIcon;

	UPROPERTY(meta = (BindWidget))
	UWidget* DedicatedTickIcon;

	UPROPERTY(meta = (BindWidget))
	UWidget* NetworkStatusActiveIcon;

	UPROPERTY(meta = (BindWidget))
	UWidget* NetworkStatusInActiveIcon;

	UPROPERTY(meta = (BindWidget))
	UWidget* OfflineModeTxt;

	UPROPERTY(meta = (BindWidget))
	UVerticalBox* RoomListBox;

	UPROPERTY(meta = (BindWidget))
	UChatUI* ChatWidget;
	
	UPROPERTY();
	TArray<URoomPlayerSlotUI*> PlayerSlotUIs;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<URoomSlotUI> RoomSlotClass;
private:
	UFUNCTION()
	void OnZombieModeClicked();
	UFUNCTION()
	void OnBombModeClicked();
	UFUNCTION()
	void OnDeathMatchModeClicked();
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
	UFUNCTION()
	void OnBackClicked();
	UFUNCTION()
	void OnPracticeBtnClicked();

	void UpdateRoomState();

	void RequestRoomList();
	void UpdateRoomList();

	void UpdateRoomSlot(int slotIdx);
	void ShowRoomUI();
	void ShowRoomListUI();
	void UpdateRoomOwner();
	void UpdateRoomHostType();
	void UpdateRoomGameMode();

	UNetworkManager* CachedNetworkManager;
	URoomManager* CachedRoomMgr;

private:
	FTimerHandle RequestRoomListTimer;
};
