// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickup/PickupItem.h"
#include "Components/ActorComponent.h"
#include "InteractComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FShowPickupMessage, const FString&)
DECLARE_MULTICAST_DELEGATE(FHidePickupMessage)

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FPSDEMO_API UInteractComponent : public UActorComponent
{
	GENERATED_BODY()
private:
	UPROPERTY()
	APickupItem* FocusedPickup;
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
	void TraceForPickup();
	void TryPickup();	
	FShowPickupMessage ShowPickupMessage;
	FHidePickupMessage HidePickupMessage;
};
