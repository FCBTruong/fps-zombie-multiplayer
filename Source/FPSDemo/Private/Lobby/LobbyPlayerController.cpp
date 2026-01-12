// Fill out your copyright notice in the Description page of Project Settings.


#include "Lobby/LobbyPlayerController.h"

#include "Blueprint/UserWidget.h"

void ALobbyPlayerController::BeginPlay()
{
    Super::BeginPlay();

    if (!IsLocalController()) return;
    if (!LobbyWidgetClass) return;

    LobbyWidget = CreateWidget<UUserWidget>(this, LobbyWidgetClass);
    if (!LobbyWidget) return;

    LobbyWidget->AddToViewport();

    FInputModeUIOnly InputMode;
    InputMode.SetWidgetToFocus(LobbyWidget->TakeWidget());
    InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

    SetInputMode(InputMode);
    bShowMouseCursor = true;
}