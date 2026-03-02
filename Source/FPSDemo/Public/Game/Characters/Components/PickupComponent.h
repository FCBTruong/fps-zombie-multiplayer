// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Game/Characters/Components/RoleGatedComponent.h"
#include "Shared/Types/ItemId.h"
#include "Game/GameManager.h"
#include "PickupComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnNewItemPickup, EItemId /*ItemId*/);
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FPSDEMO_API UPickupComponent : public URoleGatedComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPickupComponent();

protected:
	UFUNCTION(Client, Unreliable)
	void ClientNotifyItemPickup(EItemId ItemId);
public:	
	void PickupItem(class APickupItem* Item, bool AutoEquip = false);

	// Delegates
	FOnNewItemPickup OnNewItemPickup;
};
