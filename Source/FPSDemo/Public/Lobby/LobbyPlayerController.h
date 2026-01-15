// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "LobbyPlayerController.generated.h"

UENUM(BlueprintType)
enum class EUIPage : uint8
{
    None,
    Loading,
    Login,
    Lobby,
    InGameHUD
};

/**
 * 
 */
UCLASS()
class FPSDEMO_API ALobbyPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<class UUserWidget> LobbyWidgetClass;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<class UUserWidget> LoginWidgetClass;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<class UUserWidget> LoadingWidgetClass;

    void SetUIPage(EUIPage NewPage);
private:
    UPROPERTY(Transient)
    TObjectPtr<UUserWidget> ActiveWidget = nullptr;
    EUIPage CurrentUIPage = EUIPage::None;

    TSubclassOf<UUserWidget> GetWidgetClassForPage(EUIPage Page) const;
    void ApplyInputModeForPage(EUIPage Page);
	void HandleNetworkConnected();
    void HandleLoginSuccess();
};
