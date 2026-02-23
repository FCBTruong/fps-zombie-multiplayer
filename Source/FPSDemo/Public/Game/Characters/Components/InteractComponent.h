// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Game/Items/Pickup/PickupItem.h"
#include "Components/ActorComponent.h"
#include "InteractComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FShowInteractMessage, const FString&)
DECLARE_MULTICAST_DELEGATE(FHideInteractMessage)

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FPSDEMO_API UInteractComponent : public UActorComponent
{
	GENERATED_BODY()
private:
	UPROPERTY()
	TWeakObjectPtr<APickupItem> FocusedPickup;

	UPROPERTY()
	TWeakObjectPtr<ABaseCharacter> FocusedChar;
public:	
	// Sets default values for this component's properties
	UInteractComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UFUNCTION(Server, Reliable)
	void ServerTryPickup(APickupItem* Item);
	void HandlePickup_Internal(APickupItem* Item);
public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void TraceFocus();
	void TryPickup();	
	FShowInteractMessage ShowInteractMessage;
	FHideInteractMessage HideInteractMessage;
};
