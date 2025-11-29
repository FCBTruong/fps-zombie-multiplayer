// Fill out your copyright notice in the Description page of Project Settings.


#include "Controllers/MyPlayerController.h"
#include "Pickup/PickupItem.h"
#include "Game/ShooterGameState.h"
#include "Game/GameManager.h"
#include "Game/MyCheatManager.h"
#include "Blueprint/UserWidget.h"
#include "Characters/BaseCharacter.h"
#include "Game/TeamEliminationState.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"


AMyPlayerController::AMyPlayerController() { 
    CheatClass = UMyCheatManager::StaticClass(); 
}

void AMyPlayerController::Client_ReceiveItemsOnMap_Implementation(const TArray<FPickupData>& Items)
{
    UE_LOG(LogTemp, Warning, TEXT("Client_ReceiveItemsOnMap received %d items"), Items.Num());
    if (GMR) {
        GMR->OnReceivedItemsFromServer(Items);
    }
}

void AMyPlayerController::BeginPlay()
{
    Super::BeginPlay();

    GMR = GetGameInstance()->GetSubsystem<UGameManager>();
    if (!GMR)
    {
        return;
    }

    if (!IsLocalController())
    {
        return;
    }
    
    if (GMR->GlobalData->PlayerUIClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("MyPlayerController: Creating PlayerUI widget"));
        PlayerUI = CreateWidget<UPlayerUI>(this, GMR->GlobalData->PlayerUIClass);
        PlayerUI->AddToViewport(5);
		PlayerUI->CloseShop();
		bIsShopOpen = false;
    }
	BindingUI();
}


void AMyPlayerController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
	UE_LOG(LogTemp, Warning, TEXT("MyPlayerController: OnPossess called"));
}

void AMyPlayerController::OnRep_Pawn()
{
    Super::OnRep_Pawn();

    UE_LOG(LogTemp, Warning, TEXT("OnRep_Pawn: Pawn replicated and possessed"));

    if (IsLocalController())
    {
        BindingUI();
    }
}

void AMyPlayerController::BindingUI()
{
	UE_LOG(LogTemp, Warning, TEXT("MyPlayerController: BindingUI called"));
    if (!PlayerUI)
    {
        UE_LOG(LogTemp, Warning, TEXT("MyPlayerController: PlayerUI is null, cannot bind"));
        return;
	}
    PlayerUI->OnEnter();
    UE_LOG(LogTemp, Warning, TEXT("MyPlayerController: Local controller possessing pawn"));
    if (auto* Char = Cast<ABaseCharacter>(GetPawn()))
    {
        UE_LOG(LogTemp, Warning, TEXT("MyPlayerController: Binding health update for possessed character"));
        if (auto* HC = Char->FindComponentByClass<UHealthComponent>())
        {
            HC->OnHealthUpdated.AddUObject(PlayerUI, &UPlayerUI::UpdateHealth);
            PlayerUI->UpdateHealth(HC->GetHealth(), HC->GetMaxHealth());
        }
        if (auto* IC = Char->FindComponentByClass<UInteractComponent>())
        {
            IC->ShowPickupMessage.AddUObject(PlayerUI, &UPlayerUI::ShowPickupMessage);
			IC->HidePickupMessage.AddUObject(PlayerUI, &UPlayerUI::HidePickupMessage);
        }
        if (auto* WC = Char->FindComponentByClass<UWeaponComponent>())
        {
            WC->OnUpdateAmmoState.AddUObject(PlayerUI, &UPlayerUI::UpdateAmmo);
            WC->OnUpdateGrenades.AddUObject(PlayerUI, &UPlayerUI::UpdateGrenades);
            PlayerUI->UpdateGrenades(WC->GetGrenades());
            WC->OnUpdateCurrentWeapon.AddUObject(PlayerUI, &UPlayerUI::UpdateCurrentWeapon);
		}

        Char->OnHit.AddUObject(PlayerUI, &UPlayerUI::OnHit);
    }

	// get game state and bind to score updates
    ATeamEliminationState* GST = GetWorld()->GetGameState<ATeamEliminationState>();
    if (GST)
    {
        GST->OnUpdateScore.AddUObject(PlayerUI, &UPlayerUI::OnUpdateScore);
    }
}

void AMyPlayerController::ApplyFlash(const float& Strength)
{
    if (PlayerUI)
    {
        PlayerUI->ApplyFlashEffect(Strength);
    }
}

void AMyPlayerController::ToggleShop()
{
	UE_LOG(LogTemp, Warning, TEXT("Toggling shop UI"));
    if (!PlayerUI)
    {
        return;
    }
    
	bIsShopOpen = !bIsShopOpen;
    if (bIsShopOpen)
    {
        PlayerUI->OpenShop();
    }
    else
    {
        PlayerUI->CloseShop();
	}
}

void AMyPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent))
    {
        if (IA_SHOP)
        {
            EnhancedInput->BindAction(
                IA_SHOP,
                ETriggerEvent::Started,
                this,
                &AMyPlayerController::ToggleShop
            );
        }

        if (IA_ESCAPE)
        {
            EnhancedInput->BindAction(IA_ESCAPE, ETriggerEvent::Started, this, &AMyPlayerController::CloseShopIfOpen);
        }
    }
}


void AMyPlayerController::ServerBuyItem_Implementation(const EItemId Itemid)
{
    if (!GMR) {
        UE_LOG(LogTemp, Warning, TEXT("ServerBuyItem called but GMR is null"));
        return;
	}
	const UItemData* Item = GMR->GetItemDataById(Itemid);
    if (!Item) {
        UE_LOG(LogTemp, Warning, TEXT("ServerBuyItem called with null Item"));
        return;
	}
	UE_LOG(LogTemp, Warning, TEXT("ServerBuyItem called for item: %s"), *GetNameSafe(Item));
    AMyPlayerState* PS = GetPlayerState<AMyPlayerState>();
    if (!PS) {
        return;
    }

	// check inventory component whether the player can buy this item
	ABaseCharacter* MyChar = Cast<ABaseCharacter>(GetPawn());
	if (!MyChar) {
		UE_LOG(LogTemp, Warning, TEXT("ServerBuyItem: Pawn is not ABaseCharacter"));
		return;
	}

    PS->ProcessBuy(Item);
}

void AMyPlayerController::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();

    AMyPlayerState* PS = GetPlayerState<AMyPlayerState>();
    if (PS)
    {
        UE_LOG(LogTemp, Warning, TEXT("MyPlayerController: Binding money update for player state"));
        PS->OnUpdateMoney.AddUObject(PlayerUI->WBP_Shop, &UShopUI::UpdateShopMoneyStatus);
        PS->OnUpdateBoughtItems.AddUObject(PlayerUI->WBP_Shop, &UShopUI::UpdateBoughtItemsStatus);
    }
    else {
        UE_LOG(LogTemp, Warning, TEXT("MyPlayerController: PlayerState is null, cannot bind money update"));
    }
}

void AMyPlayerController::CloseShopIfOpen()
{
    if (bIsShopOpen)
    {
        ToggleShop();
    }
}

void AMyPlayerController::SetViewmodelOverlay(UMaterialInstanceDynamic* MID)
{
    if (PlayerUI)
    {
		PlayerUI->ViewmodelOverlay->SetBrushFromMaterial(MID);
    }
}