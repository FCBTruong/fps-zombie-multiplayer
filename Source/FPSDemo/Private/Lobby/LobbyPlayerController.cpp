// Fill out your copyright notice in the Description page of Project Settings.


#include "Lobby/LobbyPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Network/NetworkManager.h"
#include "Lobby/LobbyUI.h"
#include "Game/MyCheatManager.h"
#include "Lobby/PlayerInfoManager.h"

ALobbyPlayerController::ALobbyPlayerController()
{
    CheatClass = UMyCheatManager::StaticClass();
}

void ALobbyPlayerController::BeginPlay()
{
    Super::BeginPlay();

    if (!IsLocalController())
    {
        return;
    }

    // Start on Loading (or whatever you want)
    SetUIPage(EUIPage::Loading);

    UNetworkManager* NetworkManager = GetGameInstance()->GetSubsystem<UNetworkManager>();
    if (NetworkManager)
    {
        NetworkManager->OnNetworkConnected.AddUObject(
            this, &ALobbyPlayerController::HandleNetworkConnected);

        NetworkManager->OnLoginSuccess.AddUObject(
            this, &ALobbyPlayerController::HandleLoginSuccess);

        NetworkManager->OnNetworkConnectError.AddUObject(
            this, &ALobbyPlayerController::HandleNetworkConnectError);

        if (NetworkManager->IsConnected()) {
            HandleNetworkConnected();
        }
        else if (!NetworkManager->IsConnecting()){
			NetworkManager->Connect();
        }
    }
}

void ALobbyPlayerController::HandleNetworkConnected()
{
    SetUIPage(EUIPage::Login);
}

void ALobbyPlayerController::HandleNetworkConnectError()
{
    SetUIPage(EUIPage::Lobby);
}

void ALobbyPlayerController::HandleLoginSuccess()
{
    SetUIPage(EUIPage::Lobby);
}

void ALobbyPlayerController::SetUIPage(EUIPage NewPage)
{
    if (!IsLocalController())
    {
        return;
    }

    if (CurrentUIPage == NewPage)
    {
        return;
    }

    // Remove old widget
    if (ActiveWidget)
    {
        ActiveWidget->RemoveFromParent();
        ActiveWidget = nullptr;
    }

    CurrentUIPage = NewPage;

    // None => just clear UI and set game input
    if (NewPage == EUIPage::None)
    {
        FInputModeGameOnly Mode;
        SetInputMode(Mode);
        bShowMouseCursor = false;
        return;
    }

    // Create new widget
    TSubclassOf<UUserWidget> WidgetClass = GetWidgetClassForPage(NewPage);
    if (!WidgetClass)
    {
        // Fallback: if the class wasn't assigned in BP, don't crash.
        return;
    }

    ActiveWidget = CreateWidget<UUserWidget>(this, WidgetClass);
    if (!ActiveWidget)
    {
        return;
    }
    ActiveWidget->SetIsFocusable(true);
    ActiveWidget->AddToViewport();

    ApplyInputModeForPage(NewPage);
}

TSubclassOf<UUserWidget> ALobbyPlayerController::GetWidgetClassForPage(EUIPage Page) const
{
    switch (Page)
    {
    case EUIPage::Loading:   return LoadingWidgetClass;
    case EUIPage::Login:     return LoginWidgetClass;
    case EUIPage::Lobby:     return LobbyWidgetClass;
    case EUIPage::InGameHUD: return nullptr; // add HUD class if you have one
    default:                 return nullptr;
    }
}

void ALobbyPlayerController::ApplyInputModeForPage(EUIPage Page)
{
    // Example rule: UI pages use UI-only, HUD uses game-only, adjust as needed.
    const bool bIsMenuPage =
        (Page == EUIPage::Loading) ||
        (Page == EUIPage::Login) ||
        (Page == EUIPage::Lobby);

    if (bIsMenuPage && ActiveWidget)
    {
        FInputModeGameAndUI Mode;
        Mode.SetWidgetToFocus(ActiveWidget->TakeWidget());
        Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
        Mode.SetHideCursorDuringCapture(false);

        SetInputMode(Mode);
        bShowMouseCursor = true;
    }
    else
    {
        FInputModeGameOnly Mode;
        SetInputMode(Mode);
        bShowMouseCursor = false;
    }
}

void ALobbyPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();
    
    InputComponent->BindKey(EKeys::Enter, IE_Pressed, this, &ALobbyPlayerController::OnToggleChatPressed);
}

void ALobbyPlayerController::OnToggleChatPressed()
{
	UE_LOG(LogTemp, Log, TEXT("Toggle chat pressed"));
    if (ActiveWidget)
    {
        if (ULobbyUI* LobbyWidget = Cast<ULobbyUI>(ActiveWidget))
        {
            LobbyWidget->ToggleChat();
        }
	}
}

// cheat command to login with specified id
void ALobbyPlayerController::Login(int Id) {
    UPlayerInfoManager::Get(GetWorld())->Login(Id);
}

void ALobbyPlayerController::Crosshair(FString CrosshairCode) {
    UPlayerInfoManager::Get(GetWorld())->SetCrosshairCode(CrosshairCode);
	UE_LOG(LogTemp, Log, TEXT("Set CrosshairCode to: %s"), *CrosshairCode);
}