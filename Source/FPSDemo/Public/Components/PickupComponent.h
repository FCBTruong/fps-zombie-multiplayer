// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/RoleGatedComponent.h"
#include "Items/ItemIds.h"
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
	// Called when the game starts
	virtual void BeginPlay() override;
	UGameManager* GMR;

	UFUNCTION(Client, Unreliable)
	void ClientNotifyItemPickup(EItemId ItemId);
public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void PickupItem(class APickupItem* Item, bool AutoEquip = false);

	FOnNewItemPickup OnNewItemPickup;
};
