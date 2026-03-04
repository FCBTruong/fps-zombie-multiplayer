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
        game::net::NotiMessageReply MessPkg;
        if (!MessPkg.ParseFromString(Payload))
        {
			UE_LOG(LogTemp, Error, TEXT("SceneManager: NOTI_MESSAGE Failed to parse NotiMessageReply"));
            return;
        }
		FString Message = UTF8_TO_TCHAR(MessPkg.mess().c_str());

		UE_LOG(LogTemp, Log, TEXT("Received NOTI_MESSAGE: %s"), *Message);
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
	return GameInstance->GetSubsystem<USceneManager>();
}

void USceneManager::OpenPopupDialogOk(
	const FString& Message,
	TFunction<void()> OnOk
)
{
    UWorld* World = GetWorld();
	if (!World) return;
    UGameManager* GameManager = UGameManager::Get(World);
    UGlobalDataAsset* GlobalData = GameManager->GlobalData;
    if (!GlobalData->PopupDialogUIClass)
    {
        UE_LOG(LogTemp, Error, TEXT("PopupOkCancelClass not set"));
        return;
    }

    APlayerController* PC = World->GetFirstPlayerController();
	if (!PC) return;
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
    UGlobalDataAsset* GlobalData = GameManager->GlobalData;
    if (!GlobalData->PopupDialogUIClass)
    {
        UE_LOG(LogTemp, Error, TEXT("PopupOkCancelClass not set"));
        return;
    }

    UWorld* World = GetWorld();
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