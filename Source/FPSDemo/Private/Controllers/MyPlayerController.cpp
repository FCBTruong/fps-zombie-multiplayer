// Fill out your copyright notice in the Description page of Project Settings.


#include "Controllers/MyPlayerController.h"
#include "Pickup/PickupItem.h"
#include "Game/ShooterGameState.h"
#include "Game/GameManager.h"
#include "Game/MyCheatManager.h"
#include "Blueprint/UserWidget.h"
#include "Characters/BaseCharacter.h"


AMyPlayerController::AMyPlayerController() { 
    CheatClass = UMyCheatManager::StaticClass(); 
}

void AMyPlayerController::Client_ReceiveItemsOnMap_Implementation(const TArray<FPickupData>& Items)
{
    UE_LOG(LogTemp, Warning, TEXT("Client_ReceiveItemsOnMap received %d items"), Items.Num());
    UGameManager* GMR = GetGameInstance()->GetSubsystem<UGameManager>();
    if (!GMR)
    {
        return;
    }
    GMR->OnReceivedItemsFromServer(Items);
}

void AMyPlayerController::BeginPlay()
{
    Super::BeginPlay();

    if (!IsLocalController())
    {
        return;
    }
    
    UGameManager* GMR = GetGameInstance()->GetSubsystem<UGameManager>();
    if (!GMR)
    {
        return;
    }
    if (GMR->GlobalData->PlayerUIClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("MyPlayerController: Creating PlayerUI widget"));
        PlayerUI = CreateWidget<UPlayerUI>(this, GMR->GlobalData->PlayerUIClass);
        PlayerUI->AddToViewport();
    }
	BindingUI();
}


void AMyPlayerController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);


    // Only bind for our owned pawn
	UE_LOG(LogTemp, Warning, TEXT("MyPlayerController: OnPossess called"));
    if (IsLocalController())
    {
		BindingUI();
    }
}

void AMyPlayerController::BindingUI()
{
    if (!PlayerUI)
    {
        UE_LOG(LogTemp, Warning, TEXT("MyPlayerController: PlayerUI is null, cannot bind"));
        return;
	}
    UE_LOG(LogTemp, Warning, TEXT("MyPlayerController: Local controller possessing pawn"));
    if (auto* Char = Cast<ABaseCharacter>(GetPawn()))
    {
        UE_LOG(LogTemp, Warning, TEXT("MyPlayerController: Binding health update for possessed character"));
        if (auto* HC = Char->FindComponentByClass<UHealthComponent>())
        {
            HC->OnHealthUpdated.AddUObject(PlayerUI, &UPlayerUI::UpdateHealth);
        }
        if (auto* IC = Char->FindComponentByClass<UInteractComponent>())
        {
            IC->ShowPickupMessage.AddUObject(PlayerUI, &UPlayerUI::ShowPickupMessage);
        }
    }
}