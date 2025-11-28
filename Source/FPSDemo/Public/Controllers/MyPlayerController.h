// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Pickup/PickupData.h"
#include "UI/PlayerUI.h"
#include "Items/ItemIds.h"
#include "Game/GameManager.h"
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

	virtual void SetupInputComponent() override;

	UFUNCTION(Server, Reliable)
	void ServerBuyItem(const EItemId ItemId);
	void OnRep_PlayerState() override;

	void CloseShopIfOpen();
	void SetViewmodelOverlay(UMaterialInstanceDynamic* MID);
};
