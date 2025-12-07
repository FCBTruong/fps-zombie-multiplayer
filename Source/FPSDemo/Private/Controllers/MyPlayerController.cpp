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
            WC->OnUpdateCurrentWeapon.AddUObject(PlayerUI, &UPlayerUI::UpdateCurrentWeapon);
            WC->OnUpdateRifleWeapon.AddUObject(PlayerUI, &UPlayerUI::UpdateRifle);
            WC->OnUpdatePistolWeapon.AddUObject(PlayerUI, &UPlayerUI::UpdatePistol);

            WC->TriggerUpdateUI();
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

        this->bShowMouseCursor = true;
        SetIgnoreLookInput(true);
        SetIgnoreMoveInput(true);
    }
    else
    {
        PlayerUI->CloseShop();
        this->bShowMouseCursor = false;
        FInputModeGameOnly GameInput;
        this->SetInputMode(GameInput);
        this->bShowMouseCursor = false;

        SetIgnoreLookInput(false);
        SetIgnoreMoveInput(false);
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

        if (IMC_FPS)
        {
            if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
            {
                Subsystem->AddMappingContext(IMC_FPS, 0);
            }
		}

        if (IA_MOVEMENT)
        {
            EnhancedInput->BindAction(IA_MOVEMENT, ETriggerEvent::Triggered, this, &AMyPlayerController::Move);
		}

        if (IA_SHOOT)
        {
            EnhancedInput->BindAction(IA_SHOOT, ETriggerEvent::Started, this, &AMyPlayerController::OnLeftClickStart);
            EnhancedInput->BindAction(IA_SHOOT, ETriggerEvent::Completed, this, &AMyPlayerController::OnLeftClickRelease);
        }

        if (IA_JUMP)
        {
            EnhancedInput->BindAction(IA_JUMP, ETriggerEvent::Started, this, &AMyPlayerController::Jump);
        }

        if (IA_CROUCH)
        {
            EnhancedInput->BindAction(IA_CROUCH, ETriggerEvent::Started, this, &AMyPlayerController::ClickCrouch);
        }

        if (IA_CAMERA)
        {
            EnhancedInput->BindAction(IA_CAMERA, ETriggerEvent::Triggered, this, &AMyPlayerController::Look);
        }

        if (IA_AIM)
        {
            EnhancedInput->BindAction(IA_AIM, ETriggerEvent::Started, this, &AMyPlayerController::ClickAim);
        }
        if (IA_RELOAD)
        {
            EnhancedInput->BindAction(IA_RELOAD, ETriggerEvent::Started, this, &AMyPlayerController::StartReload);
        }
        if (IA_PICKUP)
        {
            // With this line, using Enhanced Input system:
            EnhancedInput->BindAction(IA_PICKUP, ETriggerEvent::Started, this, &AMyPlayerController::Pickup);
        }
        if (IA_CHANGE_VIEW)
        {
            EnhancedInput->BindAction(IA_CHANGE_VIEW, ETriggerEvent::Started, this, &AMyPlayerController::ChangeView);
        }
        if (IA_SELECT_FIRST_RIFLE)
        {
            EnhancedInput->BindAction(IA_SELECT_FIRST_RIFLE, ETriggerEvent::Started, this, &AMyPlayerController::EquipSlot, FGameConstants::SLOT_RIFLE);
        }

        if (IA_SELECT_MELEE)
        {
            EnhancedInput->BindAction(IA_SELECT_MELEE, ETriggerEvent::Started, this, &AMyPlayerController::EquipSlot, FGameConstants::SLOT_MELEE);
        }
        if (IA_SELECT_PISTOL)
        {
            EnhancedInput->BindAction(IA_SELECT_PISTOL, ETriggerEvent::Started, this, &AMyPlayerController::EquipSlot, FGameConstants::SLOT_PISTOL);
        }
        if (IA_SELECT_THROWABLE)
        {
            EnhancedInput->BindAction(IA_SELECT_THROWABLE, ETriggerEvent::Started, this, &AMyPlayerController::EquipSlot, FGameConstants::SLOT_THROWABLE);
        }
        if (IA_DROP_WEAPON)
        {
            EnhancedInput->BindAction(IA_DROP_WEAPON, ETriggerEvent::Started, this, &AMyPlayerController::DropWeapon);
        }
        if (IA_SELECT_SPIKE) {
			EnhancedInput->BindAction(IA_SELECT_SPIKE, ETriggerEvent::Started, this, &AMyPlayerController::EquipSlot, FGameConstants::SLOT_SPIKE);
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
        if (!PlayerUI)
        {
            UE_LOG(LogTemp, Warning, TEXT("MyPlayerController: PlayerUI is null, cannot bind money update"));
            return;
		}
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

void AMyPlayerController::ShowScope()
{
    if (PlayerUI)
    {
		PlayerUI->ShowScope();
	}
}

void AMyPlayerController::HideScope()
{
    if (PlayerUI)
    {
        PlayerUI->HideScope();
	}
}

void AMyPlayerController::Move(const FInputActionValue& Value)
{
    APawn* MyPawn = GetPawn();
    if (!MyPawn) return;
    FVector2D MoveInput = Value.Get<FVector2D>();

    const FRotator ControlRot = this->GetControlRotation();
    const FRotator YawRot(0, ControlRot.Yaw, 0);

    const FVector Forward = FRotationMatrix(YawRot).GetUnitAxis(EAxis::X);
    const FVector Right = FRotationMatrix(YawRot).GetUnitAxis(EAxis::Y);

    MyPawn->AddMovementInput(Forward, MoveInput.Y);
    MyPawn->AddMovementInput(Right, MoveInput.X);
}

void AMyPlayerController::OnLeftClickStart()
{
    if (bIsShopOpen) return;
    APawn* MyPawn = GetPawn();
    if (!MyPawn) return;
    if (ABaseCharacter* MyChar = Cast<ABaseCharacter>(MyPawn))
    {
        if (UWeaponComponent* WC = MyChar->FindComponentByClass<UWeaponComponent>())
        {
            WC->StartAttack();
        }
    }
}

void AMyPlayerController::OnLeftClickRelease()
{
    if (bIsShopOpen) return;
    APawn* MyPawn = GetPawn();
    if (!MyPawn) return;
    if (ABaseCharacter* MyChar = Cast<ABaseCharacter>(MyPawn))
    {
        if (UWeaponComponent* WC = MyChar->FindComponentByClass<UWeaponComponent>())
        {
            WC->StopAttack();
        }
    }
}

void AMyPlayerController::Jump() {
    APawn* MyPawn = GetPawn();
    if (!MyPawn) return;
    if (ABaseCharacter* MyChar = Cast<ABaseCharacter>(MyPawn))
    {
        MyChar->Jump();
    }
}

void AMyPlayerController::ClickCrouch() {
    APawn* MyPawn = GetPawn();
    if (!MyPawn) return;
    if (ABaseCharacter* MyChar = Cast<ABaseCharacter>(MyPawn))
    {
        MyChar->ClickCrouch();
    }
}

void AMyPlayerController::Look(const FInputActionValue& Value) {
    FVector2D Axis = Value.Get<FVector2D>();
    APawn* MyPawn = GetPawn();
    if (!MyPawn) return;
    ABaseCharacter* MyChar = Cast<ABaseCharacter>(MyPawn);
	if (!MyChar) return;

	float AimSensitivity = MyChar->GetAimSensitivity() * 0.3;

    AddYawInput(Axis.X * AimSensitivity);
    AddPitchInput(-Axis.Y * AimSensitivity);
}

void AMyPlayerController::ClickAim() {
    APawn* MyPawn = GetPawn();
    if (!MyPawn) return;
    if (ABaseCharacter* MyChar = Cast<ABaseCharacter>(MyPawn))
    {
        MyChar->ClickAim();
    }
}

void AMyPlayerController::StartReload() {
    APawn* MyPawn = GetPawn();
    if (!MyPawn) return;
    if (ABaseCharacter* MyChar = Cast<ABaseCharacter>(MyPawn))
    {
        if (UWeaponComponent* WC = MyChar->FindComponentByClass<UWeaponComponent>())
        {
            WC->StartReload();
        }
    }
}

void AMyPlayerController::Pickup() {
    APawn* MyPawn = GetPawn();
    if (!MyPawn) return;
    if (ABaseCharacter* MyChar = Cast<ABaseCharacter>(MyPawn))
    {
        if (UInteractComponent* IC = MyChar->FindComponentByClass<UInteractComponent>())
        {
            IC->TryPickup();
        }
    }
}

void AMyPlayerController::EquipSlot(const int32 SlotIndex) {
    APawn* MyPawn = GetPawn();
    if (!MyPawn) return;
    if (ABaseCharacter* MyChar = Cast<ABaseCharacter>(MyPawn))
    {
        if (UWeaponComponent* WC = MyChar->FindComponentByClass<UWeaponComponent>())
        {
            WC->EquipSlot(SlotIndex);
        }
    }
}

void AMyPlayerController::DropWeapon() {
    APawn* MyPawn = GetPawn();
    if (!MyPawn) return;
    if (ABaseCharacter* MyChar = Cast<ABaseCharacter>(MyPawn))
    {
        if (UWeaponComponent* WC = MyChar->FindComponentByClass<UWeaponComponent>())
        {
            WC->DropWeapon();
        }
    }
}

void AMyPlayerController::ChangeView() {
    APawn* MyPawn = GetPawn();
    if (!MyPawn) return;
    if (ABaseCharacter* MyChar = Cast<ABaseCharacter>(MyPawn))
    {
        MyChar->ChangeView();
    }
}