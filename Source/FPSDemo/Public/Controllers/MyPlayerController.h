// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Pickup/PickupData.h"
#include "UI/PlayerUI.h"
#include "Items/ItemIds.h"
#include "Game/GameManager.h"
#include "InputActionValue.h"
#include "MyPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class FPSDEMO_API AMyPlayerController : public APlayerController
{
	GENERATED_BODY()

private:
	bool bIsShopOpen = false;
	UGameManager* GMR;

protected:

public:
	AMyPlayerController();

	virtual void BeginPlay() override;
	UPlayerUI* PlayerUI;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	void BindingUI();
	virtual void OnRep_Pawn() override;
	void ApplyFlash(const float& Strength);
	void ToggleShop();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	class UInputAction* IA_SHOP;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	class UInputAction* IA_ESCAPE;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    class UInputAction* IA_MOVEMENT;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    class UInputAction* IA_SHOOT;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    class UInputMappingContext* IMC_FPS;

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

	virtual void SetupInputComponent() override;

	UFUNCTION(Server, Reliable)
	void ServerBuyItem(const EItemId ItemId);
	virtual void OnRep_PlayerState() override;

	void CloseShopIfOpen();
	void ShowScope();
	void HideScope();
	void HandleAimingChanged(bool bIsAiming);

	void Move(const FInputActionValue& Value);
    void OnLeftClickStart();
	void OnLeftClickRelease();
    void Jump();
    void ClickCrouch();
	void StopCrouch();
    void Look(const FInputActionValue& Value);
    void ClickAim();
	void StopAim();
	void StartReload();
	void Pickup();
	void EquipSlot(const int32 SlotIndex);
	void DropWeapon();
	void ChangeView();
	void OnButtonE_Started();
	void OnButtonE_Completed();
	void OnMouse_RightStarted();
	void StartDefuseSpike();
	void StopDefuseSpike();
	void ClickSlow();
	void ReleaseSlow();
	FName GetTeamId();
    AActor* FindLivingTeammate(AController* Spectator);
	virtual void BeginSpectatingState() override;
	virtual void EndSpectatingState() override;
    void UpdateSpectatedPawn(APawn* InPawn, bool bSpectating);

    void OnSpectateNextPressed();

    AActor* FindNextLivingTeammate(AActor* CurrentTarget) const;
    bool IsSpectatingState() const;

    UFUNCTION(Server, Reliable)
    void ServerSetSpectateTarget(bool bNext);

    UFUNCTION(Client, Reliable)
    void ClientSetSpectateViewTarget(AActor* Target, float BlendTime);

    UPROPERTY()
    AActor* CurrentSpectateTarget;
    void SetPlayerPlay();
	void SetPlayerSpectate();
	void ShowScoreboard();
	void HideScoreboard();

private:
	void HandleSpikeChanged(bool bHasSpike);
    void NotifyItemPickedUp(EItemId ItemId);
	void NotifyToastMessage(const FText& Message);
};
