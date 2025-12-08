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
public:
	AMyPlayerController();

	UFUNCTION(Client, Reliable)
	void Client_ReceiveItemsOnMap(const TArray<FPickupData>& Items);
	void BeginPlay() override;
	UPlayerUI* PlayerUI;
	void OnPossess(APawn* InPawn) override;
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
    class UInputAction* IA_RUN;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    class UInputAction* IA_CROUCH;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    class UInputAction* IA_CAMERA;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    class UInputAction* IA_SELECT_FIRST_RIFLE;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
    class UInputAction* IA_SELECT_SECOND_RIFLE;
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
    class UInputAction* IA_DEFUSE_SPIKE;

	virtual void SetupInputComponent() override;

	UFUNCTION(Server, Reliable)
	void ServerBuyItem(const EItemId ItemId);
	void OnRep_PlayerState() override;

	void CloseShopIfOpen();
	void SetViewmodelOverlay(UMaterialInstanceDynamic* MID);
	void ShowScope();
	void HideScope();

	void Move(const FInputActionValue& Value);
    void OnLeftClickStart();
	void OnLeftClickRelease();
    void Jump();
    void ClickCrouch();
    void Look(const FInputActionValue& Value);
    void ClickAim();
	void StartReload();
	void Pickup();
	void EquipSlot(const int32 SlotIndex);
	void DropWeapon();
	void ChangeView();
	void StartDefuseSpike();
	void StopDefuseSpike();
};
