// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Pickup/PickupData.h"
#include "UI/PlayerUI.h"
#include "MyPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class FPSDEMO_API AMyPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AMyPlayerController();

	UFUNCTION(Client, Reliable)
	void Client_ReceiveItemsOnMap(const TArray<FPickupData>& Items);
	void BeginPlay() override;
	UPlayerUI* PlayerUI;
	void OnPossess(APawn* InPawn) override;
	void BindingUI();
};
