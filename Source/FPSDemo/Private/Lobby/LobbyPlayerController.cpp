// Fill out your copyright notice in the Description page of Project Settings.


#include "Lobby/LobbyPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Network/NetworkManager.h"

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
    }
}

void ALobbyPlayerController::HandleNetworkConnected()
{
    SetUIPage(EUIPage::Login);
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
        FInputModeUIOnly Mode;
        Mode.SetWidgetToFocus(ActiveWidget->TakeWidget());
        Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
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