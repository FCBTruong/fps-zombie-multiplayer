// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickup/PickupItem.h"
#include "Components/ActorComponent.h"
#include "InteractComponent.generated.h"


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

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void TraceForPickup();
	void TryPickup();	
};
