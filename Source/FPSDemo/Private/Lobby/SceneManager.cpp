// Fill out your copyright notice in the Description page of Project Settings.


#include "Lobby/SceneManager.h"
#include "Game/GameManager.h"
#include "Game/GlobalDataAsset.h"
#include "Blueprint/UserWidget.h"
#include "Lobby/PopupDialogUI.h"

USceneManager* USceneManager::Get(UWorld* World)
{
	if (!World)
	{
		return nullptr;
	}
	UGameInstance* GameInstance = World->GetGameInstance();
	if (!GameInstance)
	{
		return nullptr;
	}
	return GameInstance->GetSubsystem<USceneManager>();
}

void USceneManager::OpenPopupDialogOkCancel(
	const FString& Message,
	TFunction<void()> OnOk,
	TFunction<void()> OnCancel
)
{
    UGameManager* GameManager = UGameManager::Get(GetWorld());
    if (!GameManager)
        return;

    UGlobalDataAsset* GlobalData = GameManager->GlobalData;
    if (!GlobalData->PopupDialogUIClass)
    {
        UE_LOG(LogTemp, Error, TEXT("PopupOkCancelClass not set"));
        return;
    }

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    APlayerController* PC = World->GetFirstPlayerController();
    UPopupDialogUI* Popup = CreateWidget<UPopupDialogUI>(
        PC,
        GlobalData->PopupDialogUIClass.Get()
    );
    if (!Popup) return;
    Popup->AddToViewport(100);

    Popup->Setup(
        Message,
        MoveTemp(OnOk),
        MoveTemp(OnCancel)
    );
}