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
#include "Game/SpikeMode.h"
#include "Components/HealthComponent.h"
#include "Components/InteractComponent.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/EquipComponent.h"
#include "Items/ItemConfig.h"
#include "Components/WeaponFireComponent.h"
#include "Components/WeaponMeleeComponent.h"
#include "Components/ThrowableComponent.h"
#include "Components/InventoryComponent.h"

AMyPlayerController::AMyPlayerController() { 
    CheatClass = UMyCheatManager::StaticClass(); 
}

void AMyPlayerController::BeginPlay()
{
    Super::BeginPlay();

    APlayerCameraManager* PCM = this->PlayerCameraManager;
    if (PCM)
    {
        PCM->ViewPitchMin = -60.f;
        PCM->ViewPitchMax = 60.f;
    }

    GMR = GetGameInstance()->GetSubsystem<UGameManager>();
    if (!GMR)
    {
		UE_LOG(LogTemp, Warning, TEXT("MyPlayerController: GameManager subsystem not found"));
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

        if (PendingViewmodelOverlay)
        {
            ApplyViewmodelOverlay();
        }

        BindingUI();
    }
}


void AMyPlayerController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
	UE_LOG(LogTemp, Warning, TEXT("MyPlayerController: OnPossess called"));
}


void AMyPlayerController::OnUnPossess()
{
    if (ABaseCharacter* Char = Cast<ABaseCharacter>(GetPawn()))
    {
        Char->OnAimingChanged.RemoveAll(this);
    }
    Super::OnUnPossess();
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
        if (auto* HC = Char->GetHealthComponent())
        {
            HC->OnHealthUpdated.AddUObject(PlayerUI, &UPlayerUI::UpdateHealth);
            PlayerUI->UpdateHealth(HC->GetHealth(), HC->GetMaxHealth());
        }
        if (auto* IC = Char->GetInteractComponent())
        {
            IC->ShowPickupMessage.AddUObject(PlayerUI, &UPlayerUI::ShowPickupMessage);
			IC->HidePickupMessage.AddUObject(PlayerUI, &UPlayerUI::HidePickupMessage);
        }
        if (auto* EC = Char->GetEquipComponent())
        {
            EC->OnActiveItemChanged.AddUObject(PlayerUI, &UPlayerUI::UpdateCurrentWeapon);
		}
        if (auto* IC = Char->GetInventoryComponent())
        {
            IC->OnThrowablesChanged.AddUObject(PlayerUI, &UPlayerUI::UpdateGrenades);
			PlayerUI->UpdateGrenades(IC->GetThrowables());
            IC->OnAmmoChanged.AddUObject(PlayerUI, &UPlayerUI::UpdateAmmo);
        }
      
        if (auto* WC = Char->GetWeaponComponent())
        {
            WC->OnUpdateAmmoState.AddUObject(PlayerUI, &UPlayerUI::UpdateAmmo);
            WC->OnUpdateGrenades.AddUObject(PlayerUI, &UPlayerUI::UpdateGrenades);
            WC->OnUpdateCurrentWeapon.AddUObject(PlayerUI, &UPlayerUI::UpdateCurrentWeapon);
            WC->OnUpdateRifleWeapon.AddUObject(PlayerUI, &UPlayerUI::UpdateRifle);
            WC->OnUpdatePistolWeapon.AddUObject(PlayerUI, &UPlayerUI::UpdatePistol);
			WC->OnUpdatePlantSpikeState.AddUObject(PlayerUI, &UPlayerUI::OnUpdatePlantSpikeState);
			WC->OnUpdateDefuseSpikeState.AddUObject(PlayerUI, &UPlayerUI::OnUpdateDefuseSpikeState);
			WC->OnUpdateArmor.AddUObject(PlayerUI, &UPlayerUI::UpdateArmor);
            PlayerUI->UpdateArmor(0);
			
            WC->TriggerUpdateUI();
		}

        Char->OnHit.AddUObject(PlayerUI, &UPlayerUI::OnHit);

        Char->OnAimingChanged.AddUObject(
            this,
            &AMyPlayerController::HandleAimingChanged
        );
    }

	// get game state and bind to score updates
    AShooterGameState* GST = GetWorld()->GetGameState<AShooterGameState>();
    if (GST)
    {
        GST->OnUpdateScore.AddUObject(PlayerUI, &UPlayerUI::UpdateTeamScores);
		GST->OnUpdateRoundTime.AddUObject(PlayerUI, &UPlayerUI::OnUpdateRoundTime);
		GST->OnUpdateMatchState.AddUObject(PlayerUI, &UPlayerUI::UpdateGameState);
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
    UE_LOG(LogTemp, Warning, TEXT("ObjectToggleShop address = %p"), GMR);
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
			EnhancedInput->BindAction(IA_CROUCH, ETriggerEvent::Completed, this, &AMyPlayerController::StopCrouch);
        }

        if (IA_SLOW) {
            EnhancedInput->BindAction(IA_SLOW, ETriggerEvent::Started, this, &AMyPlayerController::ClickSlow);
            EnhancedInput->BindAction(IA_SLOW, ETriggerEvent::Completed, this, &AMyPlayerController::ReleaseSlow);
        }

        if (IA_CAMERA)
        {
            EnhancedInput->BindAction(IA_CAMERA, ETriggerEvent::Triggered, this, &AMyPlayerController::Look);
        }

        if (IA_AIM)
        {
            EnhancedInput->BindAction(IA_AIM, ETriggerEvent::Started, this, &AMyPlayerController::ClickAim);
			EnhancedInput->BindAction(IA_AIM, ETriggerEvent::Completed, this, &AMyPlayerController::StopAim);
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
        if (IA_DEFUSE_SPIKE) {
			EnhancedInput->BindAction(IA_DEFUSE_SPIKE, ETriggerEvent::Started, this, &AMyPlayerController::StartDefuseSpike);
			EnhancedInput->BindAction(IA_DEFUSE_SPIKE, ETriggerEvent::Completed, this, &AMyPlayerController::StopDefuseSpike);
        }
        if (IA_SCOREBOARD) {
            EnhancedInput->BindAction(IA_SCOREBOARD, ETriggerEvent::Started, this, &AMyPlayerController::ShowScoreboard);
            EnhancedInput->BindAction(IA_SCOREBOARD, ETriggerEvent::Completed, this, &AMyPlayerController::HideScoreboard);
		}
    }
}


void AMyPlayerController::ServerBuyItem_Implementation(const EItemId Itemid)
{
    if (!GMR) {
        UE_LOG(LogTemp, Warning, TEXT("ServerBuyItem called but GMR is null"));
        return;
	}
	const UWeaponData* Item = GMR->GetWeaponDataById(Itemid);
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
	ABaseCharacter* MyChar = Cast<ABaseCharacter>(MyPawn);

	if (!MyChar) return;
	UWeaponComponent* WeaponComp = MyChar->GetWeaponComponent();
    if (WeaponComp) {
        //if (WeaponComp->IsPlantingSpike()) {
        //    return; // cannot move while planting spike
        //}
    }

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
	UE_LOG(LogTemp, Warning, TEXT("OnLeftClickStart called"));
    if (bIsShopOpen) return;

    if (IsSpectatingState())
    {
        OnSpectateNextPressed();
        return;
    }

    APawn* MyPawn = GetPawn();
    if (!MyPawn) return;
    if (ABaseCharacter* MyChar = Cast<ABaseCharacter>(MyPawn))
    {
        if (UEquipComponent* EC = MyChar->GetEquipComponent())
        {
            const UItemConfig* ActiveItemConfig = EC->GetActiveItemConfig();
            if (ActiveItemConfig && ActiveItemConfig->GetItemType() == EItemType::Firearm) {
                if (UWeaponFireComponent* WFC = MyChar->GetWeaponFireComponent()) {
					WFC->RequestStartFire();
                }
            }
            else if (ActiveItemConfig && ActiveItemConfig->GetItemType() == EItemType::Melee) {
                if (UWeaponMeleeComponent* WMC = MyChar->GetWeaponMeleeComponent()) {
					WMC->RequestMeleeAttack();
                }
			}
            else if (ActiveItemConfig && ActiveItemConfig->GetItemType() == EItemType::Throwable) {
                if (UThrowableComponent* ThrowableComp = MyChar->GetThrowableComponent()) {
                    ThrowableComp->RequestStartThrow();
                }
            }
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
        if (UEquipComponent* EC = MyChar->GetEquipComponent())
        {
            const UItemConfig* ActiveItemConfig = EC->GetActiveItemConfig();
            if (ActiveItemConfig && ActiveItemConfig->GetItemType() == EItemType::Firearm) {
                if (UWeaponFireComponent* WFC = MyChar->GetWeaponFireComponent()) {
                    WFC->RequestStopFire();
                }
            }
        }
    }
}

void AMyPlayerController::Jump() {
    APawn* MyPawn = GetPawn();
    if (!MyPawn) return;
    if (ABaseCharacter* MyChar = Cast<ABaseCharacter>(MyPawn))
    {
        MyChar->RequestJump();
    }
}

void AMyPlayerController::ClickCrouch() {
    APawn* MyPawn = GetPawn();
    if (!MyPawn) return;
    if (ABaseCharacter* MyChar = Cast<ABaseCharacter>(MyPawn))
    {
        MyChar->RequestCrouch();
    }
}

void AMyPlayerController::StopCrouch() {
    APawn* MyPawn = GetPawn();
    if (!MyPawn) return;
    if (ABaseCharacter* MyChar = Cast<ABaseCharacter>(MyPawn))
    {
        MyChar->RequestUnCrouch();
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
        MyChar->RequestStartAiming();
    }
}

void AMyPlayerController::StopAim() {
    APawn* MyPawn = GetPawn();
    if (!MyPawn) return;
    if (ABaseCharacter* MyChar = Cast<ABaseCharacter>(MyPawn))
    {
        MyChar->RequestStopAiming();
    }
}

void AMyPlayerController::StartReload() {
    APawn* MyPawn = GetPawn();
    if (!MyPawn) return;
    if (ABaseCharacter* MyChar = Cast<ABaseCharacter>(MyPawn))
    {
        if (UWeaponFireComponent* WFC = MyChar->GetWeaponFireComponent()) {
            WFC->RequestReload();
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
			// Deprecated: direct call without Enhanced Input
        }
    }
}

void AMyPlayerController::EquipSlot(const int32 SlotIndex) {
    APawn* MyPawn = GetPawn();
    if (!MyPawn) return;
    if (ABaseCharacter* MyChar = Cast<ABaseCharacter>(MyPawn))
    {
        if (UEquipComponent* EC = MyChar->GetEquipComponent())
        {
			EC->SelectSlot(SlotIndex);
        }
    }
}

void AMyPlayerController::DropWeapon() {
    APawn* MyPawn = GetPawn();
    if (!MyPawn) return;
    if (ABaseCharacter* MyChar = Cast<ABaseCharacter>(MyPawn))
    {
        if (UEquipComponent* EC = MyChar->GetEquipComponent())
        {
            EC->RequestDropItem();
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

void AMyPlayerController::StartDefuseSpike() {
    APawn* MyPawn = GetPawn();
    if (!MyPawn) return;
    if (ABaseCharacter* MyChar = Cast<ABaseCharacter>(MyPawn))
    {
        if (UWeaponComponent* WC = MyChar->FindComponentByClass<UWeaponComponent>())
        {
            WC->OnInput_StartDefuseSpike();
        }
    }
}

void AMyPlayerController::StopDefuseSpike() {
	UE_LOG(LogTemp, Warning, TEXT("StopDefuseSpike called"));
    APawn* MyPawn = GetPawn();
    if (!MyPawn) return;
    if (ABaseCharacter* MyChar = Cast<ABaseCharacter>(MyPawn))
    {
        if (UWeaponComponent* WC = MyChar->FindComponentByClass<UWeaponComponent>())
        {
            WC->OnInput_StopDefuseSpike();
        }
    }
}

FName AMyPlayerController::GetTeamId()
{
    AMyPlayerState* PS = GetPlayerState<AMyPlayerState>();
    if (PS)
    {
        return PS->GetTeamID();
    }
    return NAME_None;
}

void AMyPlayerController::ClickSlow() {
    APawn* MyPawn = GetPawn();
    if (!MyPawn) return;
    if (ABaseCharacter* MyChar = Cast<ABaseCharacter>(MyPawn))
    {
        MyChar->RequestSlowMovement(true);
    }
}

void AMyPlayerController::ReleaseSlow() {
    APawn* MyPawn = GetPawn();
    if (!MyPawn) return;
    if (ABaseCharacter* MyChar = Cast<ABaseCharacter>(MyPawn))
    {
        MyChar->RequestSlowMovement(false);
    }
}

AActor* AMyPlayerController::FindLivingTeammate(AController* Spectator)
{
    AMyPlayerState* SpectatorPS = Spectator->GetPlayerState<AMyPlayerState>();
    if (!SpectatorPS) return nullptr;

    for (APlayerState* PS : Spectator->GetWorld()->GetGameState()->PlayerArray)
    {
        AMyPlayerState* OtherPS = Cast<AMyPlayerState>(PS);
        if (!OtherPS) continue;

        if (OtherPS->GetTeamID() == SpectatorPS->GetTeamID()
            && OtherPS->IsAlive())
        {
            return OtherPS->GetPawn();
        }
    }
    return nullptr;
}

void AMyPlayerController::BeginSpectatingState()
{
	UE_LOG(LogTemp, Warning, TEXT("BeginSpectatingState called"));
    Super::BeginSpectatingState();
}

void AMyPlayerController::EndSpectatingState()
{
    Super::EndSpectatingState();
}

void AMyPlayerController::UpdateSpectatedPawn(APawn* InPawn, bool bSpectating)
{
	
}

bool AMyPlayerController::IsSpectatingState() const
{
    return IsInState(NAME_Spectating);
}

void AMyPlayerController::OnSpectateNextPressed()
{
    if (!IsSpectatingState())
        return;

    // Must ask server to pick & apply the next target in online game
    ServerSetSpectateTarget(true);
}

void AMyPlayerController::ServerSetSpectateTarget_Implementation(bool bNext)
{
    if (!PlayerState) return;
	AMyPlayerState* PlayerMyState = Cast<AMyPlayerState>(PlayerState);
	if (!PlayerMyState) return;
	if (PlayerMyState->IsAlive()) return;

    if (bNext) {
        if (!PlayerState->IsSpectator()) {
            return;
        }
    }

    if (!PlayerState->IsSpectator())
    {
        this->SetPlayerSpectate();
    }

    AActor* Target =
        bNext
        ? FindNextLivingTeammate(CurrentSpectateTarget)
        : FindNextLivingTeammate(nullptr);

    if (Target)
    {
        CurrentSpectateTarget = Target;
        ClientSetSpectateViewTarget(Target, 0.3f);
    }
    else {
        // get game mode
		ASpikeMode* GM = GetWorld()->GetAuthGameMode<ASpikeMode>();
        if (GM) {
            if (GM->GetPlantedSpike()) {
                CurrentSpectateTarget = GM->GetPlantedSpike();
                ClientSetSpectateViewTarget(CurrentSpectateTarget, 0.3f);
				return;
            }
        }
    }
}

AActor* AMyPlayerController::FindNextLivingTeammate(AActor* CurrentTarget) const
{
    UWorld* World = GetWorld();
    if (!World) return nullptr;

    AGameStateBase* GS = World->GetGameState();
    if (!GS || !PlayerState) return nullptr;

    // Replace with your team accessors
    const FName MyTeamId = CastChecked<AMyPlayerState>(PlayerState)->GetTeamID();

    TArray<AActor*> Candidates;
    Candidates.Reserve(GS->PlayerArray.Num());

    for (APlayerState* PS : GS->PlayerArray)
    {
        AMyPlayerState* MPS = Cast<AMyPlayerState>(PS);
        if (!MPS) continue;
        if (MPS->GetTeamID() != MyTeamId) continue;
        if (MPS == PlayerState) continue;

        APawn* MyPawn = MPS->GetPawn(); // Works if pawn is possessed; otherwise store pawn ref in PlayerState
        ABaseCharacter* Char = Cast<ABaseCharacter>(MyPawn);
        if (!Char) continue;
        if (!Char->IsAlive()) continue;

        Candidates.Add(Char);
    }

    if (Candidates.Num() == 0) return nullptr;

    // Stable cycling: sort by something consistent (optional but recommended)
    Candidates.Sort([](const AActor& A, const AActor& B)
        {
            return A.GetName() < B.GetName();
        });

    // If no current target, return first
    if (!CurrentTarget) return Candidates[0];

    int32 Index = Candidates.IndexOfByKey(CurrentTarget);
    int32 NextIndex = (Index >= 0) ? (Index + 1) % Candidates.Num() : 0;
    return Candidates[NextIndex];
}

void AMyPlayerController::ClientSetSpectateViewTarget_Implementation(AActor* Target, float BlendTime)
{
    if (Target)
    {
        //SetViewTargetWithBlend(Target, BlendTime);
		SetViewTarget(Target);
    }
}

void AMyPlayerController::SetPlayerSpectate()
{
    if (!HasAuthority()) return;

    // Mark as spectator for replication/UI.
    PlayerState->SetIsSpectator(true);

    // Switch gameplay state to spectating.
    ChangeState(NAME_Spectating);

    // If you want them considered "waiting to respawn".
    bPlayerIsWaiting = true;

    // Ensure the owning client transitions too.
    ClientGotoState(NAME_Spectating);
}

void AMyPlayerController::SetPlayerPlay()
{
    if (!HasAuthority()) return;

    PlayerState->SetIsSpectator(false);
    ChangeState(NAME_Playing);
    bPlayerIsWaiting = false;

    ClientGotoState(NAME_Playing);
}


void AMyPlayerController::ShowScoreboard()
{
    if (PlayerUI)
    {
        PlayerUI->ShowScoreboard(true);
    }
}

void AMyPlayerController::HideScoreboard()
{
    if (PlayerUI)
    {
        PlayerUI->ShowScoreboard(false);
    }
}

void AMyPlayerController::UpdateViewmodelCapture(bool bEnable)
{
    if (PlayerUI && PlayerUI->ViewmodelOverlay)
    {
		PlayerUI->ViewmodelOverlay->SetVisibility(bEnable ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
}

void AMyPlayerController::HandleAimingChanged(bool bIsAiming)
{
    if (PlayerUI)
    {
        if (bIsAiming)
        {
            PlayerUI->ShowScope();
        }
        else
        {
            PlayerUI->ShowScope();
		}
    }
}

void AMyPlayerController::NotifyViewmodelOverlayReady(
    UMaterialInstanceDynamic* OverlayMID)
{
    if (!IsLocalController())
    {
        return;
    }

    PendingViewmodelOverlay = OverlayMID;

    if (!PlayerUI)
    {
        return; // UI not ready yet
    }

    ApplyViewmodelOverlay();
}

void AMyPlayerController::ApplyViewmodelOverlay()
{
    if (!PlayerUI || !PendingViewmodelOverlay)
    {
        return;
    }

    if (PlayerUI->ViewmodelOverlay)
    {
        UE_LOG(LogTemp, Warning, TEXT("Setting viewmodel overlay material"));
        PlayerUI->ViewmodelOverlay->SetBrushFromMaterial(PendingViewmodelOverlay);
    }
    UpdateViewmodelCapture(true);
}
