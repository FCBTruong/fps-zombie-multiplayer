// Fill out your copyright notice in the Description page of Project Settings.


#include "Modules/Lobby/SceneManager.h"
#include "Game/GameManager.h"
#include "Shared/Data/GlobalDataAsset.h"
#include "Blueprint/UserWidget.h"
#include "Modules/Lobby/PopupDialogUI.h"
#include "Network/NetworkManager.h"
#include "Kismet/GameplayStatics.h"

void USceneManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
    Collection.InitializeDependency<UNetworkManager>();

    auto NetworkManager =
        Collection.InitializeDependency<UNetworkManager>();

    check(NetworkManager);

    NetworkManager->RegisterListener(this);
}

void USceneManager::OnPacketReceived(
    ECmdId CmdId,
    const std::string& Payload
)
{
    // Handle room-related packets here
    switch (CmdId)
    {
    case ECmdId::NOTI_MESSAGE:
    {
		UE_LOG(LogTemp, Warning, TEXT("SceneManager: NOTI_MESSAGE packet received"));
        game::net::NotiMessageReply MessPkg;
        if (!MessPkg.ParseFromString(Payload))
        {
			UE_LOG(LogTemp, Error, TEXT("SceneManager: NOTI_MESSAGE Failed to parse NotiMessageReply"));
            return;
        }
		FString Message = UTF8_TO_TCHAR(MessPkg.mess().c_str());

        // play notification sound  
		UGameManager* GMR = UGameManager::Get(GetWorld());
        if (GMR && GMR->GlobalData && GMR->GlobalData->NotificationSound)
        {
            UGameplayStatics::PlaySound2D(
                GetWorld(),
                GMR->GlobalData->NotificationSound
            );
		}

        OpenPopupDialogOk(
            Message,
            []() {}
        );
		break;
    }
    }
}

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

void USceneManager::OpenPopupDialogOk(
	const FString& Message,
	TFunction<void()> OnOk
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
        nullptr
    );
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