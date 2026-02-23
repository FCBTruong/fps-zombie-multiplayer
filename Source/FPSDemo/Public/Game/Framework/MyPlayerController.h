// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Game/Items/Pickup/PickupData.h"
#include "Shared/Types/ItemId.h"
#include "Game/GameManager.h"
#include "InputActionValue.h"
#include "Game/Types/TeamId.h"
#include "MyPlayerController.generated.h"

class UPlayerUI;
class AShooterGameState;
class AMyPlayerState;

/**
 *
 */
UCLASS()
class FPSDEMO_API AMyPlayerController : public APlayerController
{
    GENERATED_BODY()

protected:
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void OnPossess(APawn* InPawn) override;
    virtual void OnUnPossess() override;
    virtual void OnRep_Pawn() override;
    virtual void OnRep_PlayerState() override;
    virtual void SetupInputComponent() override;
    virtual void PawnLeavingGame() override;
public:
    AMyPlayerController();

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    class UInputAction* IA_SHOP;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    class UInputAction* IA_TEST;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    class UInputAction* IA_ESCAPE;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    class UInputAction* IA_MOVEMENT;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    class UInputAction* IA_SHOOT;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    class UInputMappingContext* IMC_FPS;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    class UInputMappingContext* IMC_UI;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    class UInputAction* IA_JUMP;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    class UInputAction* IA_AIM;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    class UInputAction* IA_RELOAD;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    class UInputAction* IA_SLOW;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    class UInputAction* IA_CROUCH;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    class UInputAction* IA_CAMERA;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    class UInputAction* IA_SELECT_FIRST_RIFLE;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    class UInputAction* IA_SELECT_MELEE;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    class UInputAction* IA_SELECT_PISTOL;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    class UInputAction* IA_SELECT_THROWABLE;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    class UInputAction* IA_DROP_WEAPON;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    class UInputAction* IA_PICKUP;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    class UInputAction* IA_CHANGE_VIEW;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    class UInputAction* IA_SELECT_SPIKE;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    class UInputAction* IA_BUTTON_E;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    class UInputAction* IA_SCOREBOARD;

    UPlayerUI* GetPlayerUI() const { return PlayerUI; }
    bool IsSpectatingState() const;
    void ShowScoreboard();
    void HideScoreboard();
    void RequestBuyItem(EItemId ItemId);
    void Test();
    void RequestSpectateNextPlayer();
    void BindCharacter(ABaseCharacter* Char);
    void SetTimeOfDeath(float TimeDead);
    void NotifyToastMessage(const FText& Message);
    void SetBackendUserId(int InBackendUserId) { BackendUserId = InBackendUserId; }
    void SetMouseSensitivity(float InMouseSensitivity);
    float GetMouseSensitivity() const;
    int32 GetBackendUserId() const { return BackendUserId; }
    ETeamId GetTeamId() const;
    AActor* FindNextLivingTeammate(AActor* CurrentTarget) const;

    UFUNCTION(Server, Reliable)
    void ServerTest();

    UFUNCTION(Server, Reliable)
	void ServerSpectateNextPlayer();
private:
    UPROPERTY()
    TWeakObjectPtr<AActor> CurrentSpectateTarget;

    UPROPERTY(Transient)
    TObjectPtr<UGameManager> GMR;

    UPROPERTY(Transient)
    TObjectPtr<UPlayerUI> PlayerUI;

    TWeakObjectPtr<ABaseCharacter> CachedChar;
    TWeakObjectPtr<AShooterGameState> CachedGS;
    TWeakObjectPtr<AMyPlayerState> CachedPS;
    bool bIsShopOpen = false;
    bool bInputMappingAdded = false;
    float TimeOfDeath = 0.f;
    float MouseSensitivity = 0.3f; // Default value
    int32 BackendUserId = -1;
    FTimerHandle ReloadDelayHandle;

    // Character delegate handles
    FDelegateHandle H_HealthUpdated;
    FDelegateHandle H_ShowPickup;
    FDelegateHandle H_HidePickup;
    FDelegateHandle H_AmmoChanged;
    FDelegateHandle H_ActiveItemChanged;
    FDelegateHandle H_SpikeChanged;
    FDelegateHandle H_ThrowablesChanged;
    FDelegateHandle H_RifleChanged;
    FDelegateHandle H_PistolChanged;
    FDelegateHandle H_NewItemPickup;
    FDelegateHandle H_Toast;
    FDelegateHandle H_UpdatePlantSpikeState;
    FDelegateHandle H_UpdateDefuseSpikeState;
    FDelegateHandle H_OnHit;
    FDelegateHandle H_AimingChanged;
    FDelegateHandle H_ArmorChanged;

    // GameState delegate handles
    FDelegateHandle H_UpdateScore;
    FDelegateHandle H_UpdateRoundTime;
    FDelegateHandle H_UpdateMatchState;
    FDelegateHandle H_OnGameResult;
    FDelegateHandle H_OnSwitchSide;
    FDelegateHandle H_OnUpdateRoundNumber;
    FDelegateHandle H_OnUpdateHeroPhase;
    FDelegateHandle H_OnUpdateHeroZombieCount;

    // PlayerState delegate handles
    FDelegateHandle H_UpdateMoney;
    FDelegateHandle H_UpdateBoughtItems;
	FDelegateHandle H_UpdateTeamId;
private:
    void ToggleShop();
    void HandleSpikeChanged(bool bHasSpike);
    void NotifyItemPickedUp(EItemId ItemId);
    void DropWeapon();
    void Pickup();
    void ClickAim();
    void StopAim();
    void StartReload();
    void EquipSlot(const int32 SlotIndex);
    void ChangeView();
    void OnButtonE_Started();
    void OnButtonE_Completed();
    void OnMouse_RightStarted();
    void OnMouse_RightReleased();
    void StartDefuseSpike();
    void StopDefuseSpike();
    void ClickSlow();
    void ReleaseSlow();
    void HandleAimingChanged(bool bIsAiming);
    void Move(const FInputActionValue& Value);
    void HandleEscapePressed();
    void OnLeftClickStart();
    void OnLeftClickRelease();
    void Jump();
    void ClickCrouch();
    void StopCrouch();
    void Look(const FInputActionValue& Value);
    void CloseShopIfOpen();
    void ShowScope();
    void HideScope();
    void BuyItem_Internal(EItemId ItemId);
    void UnbindCharacter(ABaseCharacter* Char);
    void RebindAll();
    void UnbindAll();
    void BindGameState(AShooterGameState* GS);
    void UnbindGameState(AShooterGameState* GS);
    void BindPlayerState(AMyPlayerState* PS);
    void UnbindPlayerState(AMyPlayerState* PS);
    void AddDefaultInputMapping();
    void RemoveDefaultInputMapping();
    void EnterUIMode();
    void EnterGameMode();
    void OnAmmoChanged(int32 Clip, int32 Reserve);
    void HandleUpdateTeamScore(int32 TeamAScore, int32 TeamBScore);
    void OnToggleChatPressed();
    void SpectateNextPlayer_Internal();
    void SetGameplayInputEnabled(bool bEnabled);
    ABaseCharacter* GetMyChar() const;

    UFUNCTION(Server, Reliable)
    void ServerBuyItem(EItemId ItemId);

    UFUNCTION(Client, Reliable)
    void ClientSpectateTarget(AActor* Target, float BlendTime = 0.f);
};
