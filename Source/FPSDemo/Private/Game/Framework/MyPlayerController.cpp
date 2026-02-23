// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/Framework/MyPlayerController.h"
#include "Game/Items/Pickup/PickupItem.h"
#include "Game/Framework/ShooterGameState.h"
#include "Game/GameManager.h"
#include "Game/Dev/MyCheatManager.h"
#include "Blueprint/UserWidget.h"
#include "Game/Characters/BaseCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Game/Modes/Spike/SpikeMode.h"
#include "Game/Characters/Components/HealthComponent.h"
#include "Game/Characters/Components/InteractComponent.h"
#include "Components/CanvasPanelSlot.h"
#include "Game/Characters/Components/EquipComponent.h"
#include "Shared/Data/Items/ItemConfig.h"
#include "Game/Characters/Components/WeaponFireComponent.h"
#include "Game/Characters/Components/WeaponMeleeComponent.h"
#include "Game/Characters/Components/ThrowableComponent.h"
#include "Game/Characters/Components/InventoryComponent.h"
#include "Game/Characters/Components/PickupComponent.h"
#include "Shared/Data/GlobalDataAsset.h"
#include "Shared/System/ItemsManager.h"
#include "Game/Characters/Components/SpikeComponent.h"
#include "Game/UI/PlayerUI.h"
#include "Game/Characters/Components/CharCameraComponent.h"
#include "Modules/Lobby/SceneManager.h"
#include "Kismet/GameplayStatics.h"
#include "Game/Modes/Practice/PracticeGameMode.h"
#include "Game/Subsystems/ActorManager.h"
#include "Game/Framework/PlayerSlot.h"
#include "Game/Modes/Spike/Spike.h"

AMyPlayerController::AMyPlayerController() { 
    CheatClass = UMyCheatManager::StaticClass(); 
}

void AMyPlayerController::BeginPlay()
{
    Super::BeginPlay();
	UE_LOG(LogTemp, Warning, TEXT("MyPlayerController: BeginPlay called"));
	EnterGameMode();

    AddDefaultInputMapping();

    APlayerCameraManager* PCM = this->PlayerCameraManager;
    if (PCM)
    {
        PCM->ViewPitchMin = -70.f;
        PCM->ViewPitchMax = 70.f;
    }

	GMR = UGameManager::Get(GetWorld());
    if (!GMR)
    {
		UE_LOG(LogTemp, Warning, TEXT("MyPlayerController: GameManager subsystem not found"));
        return;
    }

    if (!IsLocalController())
    {
        return;
    }
    
    if (GMR->GlobalData && GMR->GlobalData->PlayerUIClass.Get())
    {
        UE_LOG(LogTemp, Warning, TEXT("MyPlayerController: Creating PlayerUI widget"));
        PlayerUI = CreateWidget<UPlayerUI>(this, GMR->GlobalData->PlayerUIClass);
        if (PlayerUI) {
            PlayerUI->AddToViewport(5);
            PlayerUI->CloseShop();
            bIsShopOpen = false;
            RebindAll();
        }

        AGameModeBase* GM = GetWorld()->GetAuthGameMode();
        if (GM) {
            APracticeGameMode* PracticeGM = Cast<APracticeGameMode>(GM);
            if (PracticeGM)
            {
				PlayerUI->SetRadarVisible(false);
				PlayerUI->SetMatchInfoPnVisible(false);
            }
        }
    }
}

void AMyPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    RemoveDefaultInputMapping();
    UnbindAll();
    Super::EndPlay(EndPlayReason);
}

void AMyPlayerController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);
	UE_LOG(LogTemp, Warning, TEXT("MyPlayerController: OnPossess called"));
}


void AMyPlayerController::OnUnPossess()
{
	Super::OnUnPossess();
}


void AMyPlayerController::OnRep_Pawn()
{
    Super::OnRep_Pawn();

    UE_LOG(LogTemp, Warning, TEXT("OnRep_Pawn: Pawn replicated and possessed"));

    if (IsLocalController())
    {
		// unbind from previous character
        if (ABaseCharacter* OldChar = CachedChar.Get())
        {
            UnbindCharacter(OldChar);
            CachedChar.Reset();
		}

        if (APawn* P = GetPawn())
        {
            ABaseCharacter* Char = Cast<ABaseCharacter>(P);
            if (Char)
            {
                CachedChar = Char;
                BindCharacter(Char);
            }
		}
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

		EnterUIMode();  
    }
    else
    {
        PlayerUI->CloseShop();
        this->bShowMouseCursor = false;
        
		EnterGameMode();
	}
}

void AMyPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent))
    {
       /* if (IA_SHOP)
        {
            EnhancedInput->BindAction(
                IA_SHOP,
                ETriggerEvent::Started,
                this,
                &AMyPlayerController::ToggleShop
            );
        }*/

        if (IA_TEST)
        {
            EnhancedInput->BindAction(
                IA_TEST,
                ETriggerEvent::Started,
                this,
                &AMyPlayerController::Test
            );
        }

        if (IA_ESCAPE)
        {
            EnhancedInput->BindAction(IA_ESCAPE, ETriggerEvent::Started, this, &AMyPlayerController::HandleEscapePressed);
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
            EnhancedInput->BindAction(IA_AIM, ETriggerEvent::Started, this, &AMyPlayerController::OnMouse_RightStarted);
            EnhancedInput->BindAction(IA_AIM, ETriggerEvent::Completed, this, &AMyPlayerController::OnMouse_RightReleased);
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
        if (IA_BUTTON_E) {
			EnhancedInput->BindAction(IA_BUTTON_E, ETriggerEvent::Started, this, &AMyPlayerController::OnButtonE_Started);
			EnhancedInput->BindAction(IA_BUTTON_E, ETriggerEvent::Completed, this, &AMyPlayerController::OnButtonE_Completed);
        }
        if (IA_SCOREBOARD) {
            EnhancedInput->BindAction(IA_SCOREBOARD, ETriggerEvent::Started, this, &AMyPlayerController::ShowScoreboard);
            EnhancedInput->BindAction(IA_SCOREBOARD, ETriggerEvent::Completed, this, &AMyPlayerController::HideScoreboard);
		}

        InputComponent->BindKey(EKeys::Enter, IE_Pressed, this, &AMyPlayerController::OnToggleChatPressed);
    }
}


void AMyPlayerController::ServerBuyItem_Implementation(EItemId ItemId)
{
	BuyItem_Internal(ItemId);
}

void AMyPlayerController::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();

    if (IsLocalController())
    {
        if (CachedPS.IsValid()) {
            UnbindPlayerState(CachedPS.Get());
			CachedPS.Reset();
        }
        if (AMyPlayerState* PS = Cast<AMyPlayerState>(PlayerState))
        {
            CachedPS = PS;
            BindPlayerState(PS);
        }
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
    ABaseCharacter* MyChar = GetMyChar();

	if (!MyChar) return;
   
    FVector2D MoveInput = Value.Get<FVector2D>();

    const FRotator ControlRot = this->GetControlRotation();
    const FRotator YawRot(0, ControlRot.Yaw, 0);

    const FVector Forward = FRotationMatrix(YawRot).GetUnitAxis(EAxis::X);
    const FVector Right = FRotationMatrix(YawRot).GetUnitAxis(EAxis::Y);

    MyChar->AddMovementInput(Forward, MoveInput.Y);
    MyChar->AddMovementInput(Right, MoveInput.X);
}

void AMyPlayerController::OnLeftClickStart()
{
	UE_LOG(LogTemp, Warning, TEXT("OnLeftClickStart called"));
    if (bIsShopOpen) return;

    if (IsSpectatingState())
    {
		// prevent accidental click if just died
		auto CurTime = GetWorld()->GetTimeSeconds();
        if (CurTime - TimeOfDeath < 1) {
            return;
		}
		RequestSpectateNextPlayer();
        return;
    }

    ABaseCharacter* MyChar = GetMyChar();
    if (!MyChar) {
        return;
    }
    MyChar->RequestPrimaryActionPressed();
}

void AMyPlayerController::OnLeftClickRelease()
{
    if (bIsShopOpen) return;
    ABaseCharacter* MyChar = GetMyChar();
    if (!MyChar) {
        return;
    }
    MyChar->RequestPrimaryActionReleased();
}

void AMyPlayerController::Jump() {
    ABaseCharacter* MyChar = GetMyChar();
    if (!MyChar) {
        return;
    }
    MyChar->RequestJump();
}

void AMyPlayerController::ClickCrouch() {
    ABaseCharacter* MyChar = GetMyChar();
    if (!MyChar) {
        return;
    }
    MyChar->RequestCrouch();
}

void AMyPlayerController::StopCrouch() {
    ABaseCharacter* MyChar = GetMyChar();
    if (!MyChar) {
        return;
    }
    MyChar->RequestUnCrouch();
}

void AMyPlayerController::Look(const FInputActionValue& Value) {
    FVector2D Axis = Value.Get<FVector2D>();

    if (ASpike* Spike = Cast<ASpike>(GetViewTarget()))
    {
        Spike->AddCameraYaw(Axis.X);
        return;
    }
    if (IsSpectatingState()) {
        return;
    }

    ABaseCharacter* MyChar = GetMyChar();
    if (!MyChar) {
        return;
    }
    if (!MyChar) return;
	float AimSensitivity = MyChar->GetAimSensitivity() * MouseSensitivity;

    AddYawInput(Axis.X * AimSensitivity);
    AddPitchInput(-Axis.Y * AimSensitivity);
}

void AMyPlayerController::OnMouse_RightStarted() {
    ABaseCharacter* MyChar = GetMyChar();
    if (!MyChar) {
        return;
    }
    MyChar->RequestSecondaryActionPressed();
}

void AMyPlayerController::OnMouse_RightReleased() {
    ABaseCharacter* MyChar = GetMyChar();
    if (!MyChar) {
        return;
    }
    MyChar->RequestSecondaryActionReleased();
}


void AMyPlayerController::ClickAim() {
    ABaseCharacter* MyChar = GetMyChar();
    if (!MyChar) {
        return;
    }
    if (MyChar->IsAiming()) {
        MyChar->RequestStopAiming();
    }
    else {
        MyChar->RequestStartAiming();
    }
}

void AMyPlayerController::StopAim() {
    ABaseCharacter* MyChar = GetMyChar();
    if (!MyChar) {
        return;
    }
    MyChar->RequestStopAiming();
}

void AMyPlayerController::StartReload() {
    ABaseCharacter* MyChar = GetMyChar();
    if (!MyChar) {
        return;
    }
    MyChar->RequestReloadPressed();
}

void AMyPlayerController::Pickup() {
    ABaseCharacter* MyChar = GetMyChar();
    if (!MyChar) {
        return;
    }
    if (UInteractComponent* IC = MyChar->FindComponentByClass<UInteractComponent>())
    {
		UE_LOG(LogTemp, Warning, TEXT("Pickup: TryPickup called"));
		IC->TryPickup();
    }
}

void AMyPlayerController::EquipSlot(const int32 SlotIndex) {
    ABaseCharacter* MyChar = GetMyChar();
    if (!MyChar) {
        return;
    }
    if (UEquipComponent* EC = MyChar->GetEquipComponent())
    {
        EC->SelectSlot(SlotIndex);
    }
}

void AMyPlayerController::DropWeapon() {
    ABaseCharacter* MyChar = GetMyChar();
    if (!MyChar) {
        return;
    }
    if (UEquipComponent* EC = MyChar->GetEquipComponent())
    {
        EC->RequestDropItem();
    }
}

void AMyPlayerController::ChangeView() {
    ABaseCharacter* MyChar = GetMyChar();
    if (!MyChar) {
        return;
    }
    MyChar->ChangeView();
}

void AMyPlayerController::StartDefuseSpike() {
    ABaseCharacter* MyChar = GetMyChar();
    if (!MyChar) {
        return;
    }
    if (USpikeComponent* SC = MyChar->GetSpikeComponent())
    {
        SC->RequestStartDefuseSpike();
    }
}

void AMyPlayerController::StopDefuseSpike() {
    UE_LOG(LogTemp, Warning, TEXT("StopDefuseSpike called"));
    ABaseCharacter* MyChar = GetMyChar();
    if (!MyChar) {
        return;
    }
    if (USpikeComponent* SC = MyChar->GetSpikeComponent())
    {
        SC->RequestStopDefuseSpike();
    }
}

ETeamId AMyPlayerController::GetTeamId() const
{
    AMyPlayerState* PS = GetPlayerState<AMyPlayerState>();
    if (PS)
    {
        return PS->GetTeamId();
    }
    return ETeamId::None;
}

void AMyPlayerController::ClickSlow() {
    ABaseCharacter* MyChar = GetMyChar();
    if (!MyChar) {
        return;
    }
    MyChar->RequestSlowMovement(true);
}

void AMyPlayerController::ReleaseSlow() {
    ABaseCharacter* MyChar = GetMyChar();
    if (!MyChar) {
        return;
    }
    MyChar->RequestSlowMovement(false);
}

bool AMyPlayerController::IsSpectatingState() const
{
    return PlayerState && PlayerState->IsSpectator();
}

void AMyPlayerController::RequestSpectateNextPlayer()
{
    if (HasAuthority())
    {
        SpectateNextPlayer_Internal();
        return;
	}
    else {
        ServerSpectateNextPlayer();
	}
}

void AMyPlayerController::ServerSpectateNextPlayer_Implementation()
{
    SpectateNextPlayer_Internal();
}

void AMyPlayerController::SpectateNextPlayer_Internal()
{
    UE_LOG(LogTemp, Warning, TEXT("SpectateNextPlayer_Internal called"));
    if (!IsSpectatingState()) {
        return;
    }
    
	ABaseCharacter* Char = Cast<ABaseCharacter>(CurrentSpectateTarget.Get());
    if (IsValid(Char))
    {
        // make sure current target is still alive
        if (Char->IsDead())
        {
            CurrentSpectateTarget = nullptr;
        }
    }

    auto Target = FindNextLivingTeammate(CurrentSpectateTarget.Get());
    UE_LOG(LogTemp, Warning, TEXT("SpectateNextPlayer_Internal: debug1"));

    if (Target)
    {
        CurrentSpectateTarget = Target;
        UE_LOG(LogTemp, Warning, TEXT("SpectateNextPlayer_Internal: debug2"));
    }
    else {
        UE_LOG(LogTemp, Warning, TEXT("SpectateNextPlayer_Internal: debug3"));
        // get game state
		AShooterGameState* GS = GetWorld()->GetGameState<AShooterGameState>();
		
        if (GS && GS->GetMatchMode() == EMatchMode::Spike) {
            // no living teammates, spectate spike
            ASpike* Spike = GS->GetPlantedSpike();
            if (Spike) {
                CurrentSpectateTarget = Spike;
                UE_LOG(LogTemp, Warning, TEXT("SpectateNextPlayer_Internal: Spectating Spike"));
			}
        }
    }
    if (!CurrentSpectateTarget.IsValid()) {
		AActorManager* AM = AActorManager::Get(GetWorld());
		CurrentSpectateTarget = AM->GetGlobalCamera();
	}

	ClientSpectateTarget(CurrentSpectateTarget.Get());
}

AActor* AMyPlayerController::FindNextLivingTeammate(AActor* CurrentTarget) const
{
    UWorld* World = GetWorld();
    if (!World) return nullptr;

	AShooterGameState* GS = World->GetGameState<AShooterGameState>();
	if (!GS) return nullptr;

	AMyPlayerState* PS = Cast<AMyPlayerState>(PlayerState);
	if (!PS) return nullptr;
    // Replace with your team accessors
    auto MyTeamId = PS->GetTeamId();

    TArray<AActor*> Candidates;
    
    for (APlayerSlot* Slot : GS->Slots)
    {
        if (Slot->GetTeamId() != MyTeamId) continue;
		APawn* OwnedPawn = Slot->GetPawn();
		if (!IsValid(OwnedPawn)) continue;
        ABaseCharacter* Char = Cast<ABaseCharacter>(OwnedPawn);
        if (!Char) continue;
        if (!Char->IsAlive()) continue;

		if (Char == this->GetPawn()) continue; // skip self

        Candidates.Add(Char);
    }

    // log size
    UE_LOG(LogTemp, Warning, TEXT("FindNextLivingTeammate: Found %d candidates"), Candidates.Num());

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
            PlayerUI->HideScope();
		}
    }
}

void AMyPlayerController::HandleSpikeChanged(bool bHasSpike)
{
    if (!PlayerUI) {
        return;
	}
    if (bHasSpike)
    {
        PlayerUI->ShowNotiToast(
            FText::FromString(TEXT("You picked up Spike")));
    }
	PlayerUI->UpdateSpikeSlot(bHasSpike);
}

void AMyPlayerController::NotifyItemPickedUp(EItemId ItemId)
{
    if (PlayerUI)
    {
        const UItemConfig* Item = UItemsManager::Get(GetWorld())->GetItemById(ItemId);
        if (Item)
        {
            PlayerUI->ShowNotiToast(Item->DisplayName);
        }
	}
}

void AMyPlayerController::NotifyToastMessage(const FText& Message) {
    if (PlayerUI)
    {
        PlayerUI->ShowNotiToast(Message);
	}
}

void AMyPlayerController::OnButtonE_Started()
{
    // Get game mode
    AShooterGameState* GS = GetWorld()->GetGameState<AShooterGameState>();
    if (!GS) return;

    ABaseCharacter* MyChar = GetMyChar();
    if (!MyChar) {
        return;
    }

    if (GS->GetMatchMode() == EMatchMode::Spike) {
        StartDefuseSpike();
    }
    else {
        MyChar->RequestBecomeHero();
    }
}

void AMyPlayerController::OnButtonE_Completed()
{
    AShooterGameState* GS = GetWorld()->GetGameState<AShooterGameState>();
    if (!GS) return;

    if (GS->GetMatchMode() == EMatchMode::Spike) {
        StopDefuseSpike();
    }
}

void AMyPlayerController::RequestBuyItem(EItemId ItemId)
{
    if (!HasAuthority()) {
        ServerBuyItem(ItemId);
    }
    else {
		BuyItem_Internal(ItemId);
    }
}

void AMyPlayerController::BuyItem_Internal(EItemId Itemid)
{
    const UItemConfig* Item = UItemsManager::Get(GetWorld())->GetItemById(Itemid);
    if (!Item) {
        UE_LOG(LogTemp, Warning, TEXT("BuyItem_Internal called with null Item"));
        return;
    }
    UE_LOG(LogTemp, Warning, TEXT("BuyItem_Internal called for item: %s"), *GetNameSafe(Item));
    AMyPlayerState* PS = GetPlayerState<AMyPlayerState>();
    if (!PS) {
        return;
    }
    // check inventory component whether the player can buy this item
    ABaseCharacter* MyChar = Cast<ABaseCharacter>(GetPawn());
    if (!MyChar) {
        UE_LOG(LogTemp, Warning, TEXT("BuyItem_Internal: Pawn is not ABaseCharacter"));
        return;
    }
    PS->ProcessBuy(Item);
}

void AMyPlayerController::RebindAll()
{
    if (!IsLocalController() || !PlayerUI)
    {
        return;
    }

    UnbindAll();

    ABaseCharacter* Char = Cast<ABaseCharacter>(GetPawn());
    AShooterGameState* GS = GetWorld() ? GetWorld()->GetGameState<AShooterGameState>() : nullptr;
    AMyPlayerState* PS = GetPlayerState<AMyPlayerState>();

    CachedChar = Char;
    CachedGS = GS;
    CachedPS = PS;

    PlayerUI->OnEnter();

    if (Char) BindCharacter(Char);
    if (GS)   BindGameState(GS);
    if (PS)   BindPlayerState(PS);
}

void AMyPlayerController::UnbindAll()
{
    if (ABaseCharacter* Char = CachedChar.Get())
    {
        UnbindCharacter(Char);
    }
    if (AShooterGameState* GS = CachedGS.Get())
    {
        UnbindGameState(GS);
    }
    if (AMyPlayerState* PS = CachedPS.Get())
    {
        UnbindPlayerState(PS);
    }

    CachedChar.Reset();
    CachedGS.Reset();
    CachedPS.Reset();
}

void AMyPlayerController::BindCharacter(ABaseCharacter* Char)
{
    if (!Char || !PlayerUI) return;
    if (CachedChar.IsValid()) {
		// unbind previous
		UnbindCharacter(CachedChar.Get());
        CachedChar.Reset();
    }

    if (UHealthComponent* HC = Char->GetHealthComponent())
    {
        H_HealthUpdated = HC->OnHealthUpdated.AddUObject(PlayerUI, &UPlayerUI::UpdateHealth);
        PlayerUI->UpdateHealth(HC->GetHealth(), HC->GetMaxHealth());
    }

    if (UInteractComponent* IC = Char->GetInteractComponent())
    {
        H_ShowPickup = IC->ShowPickupMessage.AddUObject(PlayerUI, &UPlayerUI::ShowPickupMessage);
        H_HidePickup = IC->HidePickupMessage.AddUObject(PlayerUI, &UPlayerUI::HidePickupMessage);
    }

    if (UEquipComponent* EC = Char->GetEquipComponent())
    {
        H_AmmoChanged = EC->OnAmmoChanged.AddUObject(this, &AMyPlayerController::OnAmmoChanged);
		int CurrentAmmo, ReservedAmmo;
		EC->GetCurrentAmmo(CurrentAmmo, ReservedAmmo);
		PlayerUI->UpdateAmmo(CurrentAmmo, ReservedAmmo);

        H_ActiveItemChanged = EC->OnActiveItemChanged.AddUObject(PlayerUI, &UPlayerUI::UpdateCurrentWeapon);
		EItemId ActiveItemId = EC->GetActiveItemId();
		PlayerUI->UpdateCurrentWeapon(ActiveItemId);
    }

    if (UInventoryComponent* Inv = Char->GetInventoryComponent())
    {
        H_SpikeChanged = Inv->OnSpikeChanged.AddUObject(this, &AMyPlayerController::HandleSpikeChanged);
        H_ThrowablesChanged = Inv->OnThrowablesChanged.AddUObject(PlayerUI, &UPlayerUI::UpdateGrenades);
		H_RifleChanged = Inv->OnRifleChanged.AddUObject(PlayerUI, &UPlayerUI::UpdateRifle);
		H_PistolChanged = Inv->OnPistolChanged.AddUObject(PlayerUI, &UPlayerUI::UpdatePistol);
        PlayerUI->UpdateGrenades(Inv->GetThrowables());
		PlayerUI->UpdateRifle(Inv->GetRifleId());
		PlayerUI->UpdatePistol(Inv->GetPistolId());

		H_ArmorChanged = Inv->OnArmorChanged.AddUObject(PlayerUI, &UPlayerUI::UpdateArmor);
		PlayerUI->UpdateArmor(Inv->GetArmorPoints(), Inv->GetArmorMaxPoints());
		HandleSpikeChanged(Inv->HasSpike());
    }

    if (UPickupComponent* PC = Char->GetPickupComponent())
    {
        H_NewItemPickup = PC->OnNewItemPickup.AddUObject(this, &AMyPlayerController::NotifyItemPickedUp);
    }

    if (USpikeComponent* SC = Char->GetSpikeComponent())
    {
        H_Toast = SC->OnNotifyToastMessage.AddUObject(this, &AMyPlayerController::NotifyToastMessage);
        H_UpdatePlantSpikeState = SC->OnUpdatePlantSpikeState.AddUObject(PlayerUI, &UPlayerUI::OnUpdatePlantSpikeState);
        H_UpdateDefuseSpikeState = SC->OnUpdateDefuseSpikeState.AddUObject(PlayerUI, &UPlayerUI::OnUpdateDefuseSpikeState);
    }

    H_OnHit = Char->OnHit.AddUObject(PlayerUI, &UPlayerUI::OnHit);

    if (UCharCameraComponent* CamComp = Char->GetCharCameraComponent())
    {
        H_AimingChanged = CamComp->OnAimingVisualsChanged.AddUObject(this, &AMyPlayerController::HandleAimingChanged);
	}

	PlayerUI->UpdatePlayerName(Char->GetPlayerName());
}

void AMyPlayerController::UnbindCharacter(ABaseCharacter* Char)
{
    if (!Char) return;

    if (UHealthComponent* HC = Char->GetHealthComponent())
    {
        HC->OnHealthUpdated.Remove(H_HealthUpdated);
        H_HealthUpdated.Reset();
    }

    if (UInteractComponent* IC = Char->GetInteractComponent())
    {
        IC->ShowPickupMessage.Remove(H_ShowPickup);
        IC->HidePickupMessage.Remove(H_HidePickup);
        H_ShowPickup.Reset();
        H_HidePickup.Reset();
    }

    if (UEquipComponent* EC = Char->GetEquipComponent())
    {
        EC->OnAmmoChanged.Remove(H_AmmoChanged);
        EC->OnActiveItemChanged.Remove(H_ActiveItemChanged);
        H_AmmoChanged.Reset();
        H_ActiveItemChanged.Reset();
    }

    if (UInventoryComponent* Inv = Char->GetInventoryComponent())
    {
        Inv->OnSpikeChanged.Remove(H_SpikeChanged);
        Inv->OnThrowablesChanged.Remove(H_ThrowablesChanged);
		Inv->OnArmorChanged.Remove(H_ArmorChanged);
        H_SpikeChanged.Reset();
        H_ThrowablesChanged.Reset();
		H_ArmorChanged.Reset();
		Inv->OnRifleChanged.Remove(H_RifleChanged);
		Inv->OnPistolChanged.Remove(H_PistolChanged);
		H_RifleChanged.Reset();
		H_PistolChanged.Reset();
    }

    if (UPickupComponent* PC = Char->GetPickupComponent())
    {
        PC->OnNewItemPickup.Remove(H_NewItemPickup);
        H_NewItemPickup.Reset();
    }

    if (USpikeComponent* SC = Char->GetSpikeComponent())
    {
        SC->OnNotifyToastMessage.Remove(H_Toast);
        SC->OnUpdatePlantSpikeState.Remove(H_UpdatePlantSpikeState);
        SC->OnUpdateDefuseSpikeState.Remove(H_UpdateDefuseSpikeState);
        H_Toast.Reset();
        H_UpdatePlantSpikeState.Reset();
        H_UpdateDefuseSpikeState.Reset();
    }

    Char->OnHit.Remove(H_OnHit);

    if (UCharCameraComponent* CamComp = Char->GetCharCameraComponent())
    {
		CamComp->OnAimingVisualsChanged.Remove(H_AimingChanged);
    }
    H_OnHit.Reset();
    H_AimingChanged.Reset();
}

void AMyPlayerController::BindGameState(AShooterGameState* GS)
{
    if (!GS || !PlayerUI) return;

    H_UpdateScore = GS->OnUpdateScore.AddUObject(this, &AMyPlayerController::HandleUpdateTeamScore);
    H_UpdateRoundTime = GS->OnUpdateRoundTime.AddUObject(PlayerUI, &UPlayerUI::OnUpdateRoundTime);
    H_UpdateMatchState = GS->OnUpdateMatchState.AddUObject(PlayerUI, &UPlayerUI::UpdateGameState);
	H_OnGameResult = GS->OnGameResult.AddUObject(PlayerUI, &UPlayerUI::ShowGameResult);
	H_OnSwitchSide = GS->OnSwitchSide.AddUObject(PlayerUI, &UPlayerUI::OnSwitchSide);
	H_OnUpdateRoundNumber = GS->OnUpdateRoundNumber.AddUObject(PlayerUI, &UPlayerUI::UpdateRoundNumber);
	H_OnUpdateHeroPhase = GS->OnUpdateHeroPhase.AddUObject(PlayerUI, &UPlayerUI::UpdateHeroPhase);
	H_OnUpdateHeroZombieCount = GS->OnUpdateHeroZombieCount.AddUObject(PlayerUI, &UPlayerUI::UpdateHeroZombieNum);

    HandleUpdateTeamScore(GS->GetTeamAScore(), GS->GetTeamBScore());
    PlayerUI->OnUpdateRoundTime(GS->GetRemainingRoundTime());
	PlayerUI->UpdateGameState(GS->GetMatchState());
    PlayerUI->UpdateHeroPhase();
}

void AMyPlayerController::UnbindGameState(AShooterGameState* GS)
{
    if (!GS) return;

    GS->OnUpdateScore.Remove(H_UpdateScore);
    GS->OnUpdateRoundTime.Remove(H_UpdateRoundTime);
    GS->OnUpdateMatchState.Remove(H_UpdateMatchState);
	GS->OnGameResult.Remove(H_OnGameResult);
	GS->OnSwitchSide.Remove(H_OnSwitchSide);
	GS->OnUpdateRoundNumber.Remove(H_OnUpdateRoundNumber);
	GS->OnUpdateHeroPhase.Remove(H_OnUpdateHeroPhase);
	GS->OnUpdateHeroZombieCount.Remove(H_OnUpdateHeroZombieCount);

    H_UpdateScore.Reset();
    H_UpdateRoundTime.Reset();
    H_UpdateMatchState.Reset();
	H_OnGameResult.Reset();
	H_OnSwitchSide.Reset();
	H_OnUpdateRoundNumber.Reset();
	H_OnUpdateHeroPhase.Reset();
    H_OnUpdateHeroZombieCount.Reset();
}

void AMyPlayerController::BindPlayerState(AMyPlayerState* PS)
{
    if (!PS || !PlayerUI || !PlayerUI->WBP_Shop) return;

    H_UpdateMoney = PS->OnUpdateMoney.AddUObject(PlayerUI->WBP_Shop, &UShopUI::UpdateShopMoneyStatus);
	H_UpdateTeamId = PS->OnUpdateTeamId.AddUObject(PlayerUI, &UPlayerUI::UpdateTeamId);
    H_UpdateBoughtItems = PS->OnUpdateBoughtItems.AddUObject(PlayerUI->WBP_Shop, &UShopUI::UpdateBoughtItemsStatus);

	PlayerUI->UpdateTeamId(PS->GetTeamId());
	PlayerUI->UpdateCrosshairCode(PS->GetCrosshairCode());
}

void AMyPlayerController::UnbindPlayerState(AMyPlayerState* PS)
{
    if (!PS) return;

    PS->OnUpdateMoney.Remove(H_UpdateMoney);
    PS->OnUpdateBoughtItems.Remove(H_UpdateBoughtItems);
	PS->OnUpdateTeamId.Remove(H_UpdateTeamId);

    H_UpdateMoney.Reset();
    H_UpdateBoughtItems.Reset();
	H_UpdateTeamId.Reset();
}

ABaseCharacter* AMyPlayerController::GetMyChar() const
{
    return Cast<ABaseCharacter>(GetPawn());
}

void AMyPlayerController::AddDefaultInputMapping()
{
    if (!IsLocalController() || bInputMappingAdded || !IMC_FPS) return;

    if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
        ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        Subsystem->AddMappingContext(IMC_FPS, 0);
        bInputMappingAdded = true;
    }
}

void AMyPlayerController::RemoveDefaultInputMapping()
{
    if (!IsLocalController() || !bInputMappingAdded || !IMC_FPS) return;

    if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
        ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
    {
        Subsystem->RemoveMappingContext(IMC_FPS);
        bInputMappingAdded = false;
    }
}

void AMyPlayerController::Test() {
    ServerTest();
}

void AMyPlayerController::ServerTest_Implementation()
{
    SpectateNextPlayer_Internal();
}

void AMyPlayerController::HandleEscapePressed()
{
	UE_LOG(LogTemp, Warning, TEXT("HandleEscapePressed called"));
    if (bIsShopOpen)
    {
        ToggleShop();
        return;
    }

	AShooterGameState* GS = GetWorld()->GetGameState<AShooterGameState>();
    if (GS && !GS->CanQuitMidMatch()) {
		UE_LOG(LogTemp, Warning, TEXT("Cannot open settings during this match phase"));
        if (PlayerUI->IsSettingsOpen())
        {
            PlayerUI->ShowSettings(false);
            EnterGameMode();
        }
        else
        {
            PlayerUI->ShowSettings(true);
            EnterUIMode();
        }
        return;
    }

	// Show pause menu or other UI here
	USceneManager* SceneMgr = USceneManager::Get(GetWorld());
    if (SceneMgr)
    {
        UWorld* World = GetWorld();
        EnterUIMode();
        USceneManager::Get(GetWorld())->OpenPopupDialogOkCancel(
            TEXT("Are you sure you want to quit?"),
            [World]()
            {
                // OK pressed
                UGameplayStatics::OpenLevel(
                    World,
                    FGameConstants::LEVEL_LOBBY
                );
                UE_LOG(LogTemp, Log, TEXT("OK"));
            },
            [this]()
            {
				EnterGameMode();
                // Cancel pressed
                UE_LOG(LogTemp, Log, TEXT("Cancel"));
            }
        );
	}
}

void AMyPlayerController::EnterUIMode()
{
    bShowMouseCursor = true;
    FInputModeGameAndUI InputMode;
    InputMode.SetHideCursorDuringCapture(false);
    InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    SetInputMode(InputMode);

    SetIgnoreLookInput(true);
    SetIgnoreMoveInput(true);

    SetGameplayInputEnabled(false);
}

void AMyPlayerController::EnterGameMode()
{
    bShowMouseCursor = false;
    SetInputMode(FInputModeGameOnly());

    SetIgnoreLookInput(false);
    SetIgnoreMoveInput(false);

	SetGameplayInputEnabled(true);
}

void AMyPlayerController::OnAmmoChanged(int32 Clip, int32 Reserve) // for current active weapon
{
	PlayerUI->UpdateAmmo(Clip, Reserve);
    UE_LOG(LogTemp, Warning, TEXT("AMyPlayerController::OnAmmoChanged: Clip=%d, Reserve=%d"), Clip, Reserve);
    if (Clip <= 0 && Reserve > 0)
    {
        GetWorldTimerManager().ClearTimer(ReloadDelayHandle);

        GetWorldTimerManager().SetTimer(
            ReloadDelayHandle,
            this,
            &AMyPlayerController::StartReload,
            0.5f,
            false
        );
    }
    else {
        GetWorldTimerManager().ClearTimer(ReloadDelayHandle);
    }
}

void AMyPlayerController::ClientSpectateTarget_Implementation(AActor* Target, float BlendTime)
{
    if (!IsValid(Target))
    {
        return;
    }

    // This is the only thing you said you want: view through teammate.
    SetViewTargetWithBlend(Target, BlendTime);
}

void AMyPlayerController::SetTimeOfDeath(float Time)
{
    TimeOfDeath = Time;
}

void AMyPlayerController::PawnLeavingGame() {
    // handle by game mode
}

void AMyPlayerController::HandleUpdateTeamScore(int32 TeamAScore, int32 TeamBScore)
{
    // get my team id
	AMyPlayerState* PS = GetPlayerState<AMyPlayerState>();
	if (!PS) return;
	ETeamId MyTeamId = PS->GetTeamId();

	int FirstScore = TeamAScore;
	int SecondScore = TeamBScore;
    if (MyTeamId == ETeamId::Attacker) {
		FirstScore = TeamAScore; // because score A is for attackers
		SecondScore = TeamBScore;
    }
	else if (MyTeamId == ETeamId::Defender) {
        FirstScore = TeamBScore; // because score B is for defenders
		SecondScore = TeamAScore;
	}

    if (PlayerUI)
    {
        PlayerUI->UpdateTeamScores(FirstScore, SecondScore);
    }
}

void AMyPlayerController::OnToggleChatPressed()
{
    if (PlayerUI)
    {
        PlayerUI->ToggleChatInput();
    }
}

void AMyPlayerController::SetGameplayInputEnabled(bool bEnabled)
{
    if (!IsLocalController()) return;

    UEnhancedInputLocalPlayerSubsystem* Subsystem =
        ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
    if (!Subsystem) return;

    if (bEnabled)
    {
        if (IMC_UI)  Subsystem->RemoveMappingContext(IMC_UI);
        if (IMC_FPS) Subsystem->AddMappingContext(IMC_FPS, 0);
    }
    else
    {
        if (IMC_FPS) Subsystem->RemoveMappingContext(IMC_FPS);
        if (IMC_UI)  Subsystem->AddMappingContext(IMC_UI, 10); // UI priority cao
    }
}

void AMyPlayerController::SetMouseSensitivity(float Sensitivity)
{
    MouseSensitivity = Sensitivity;
}

float AMyPlayerController::GetMouseSensitivity() const
{
    return MouseSensitivity;
}