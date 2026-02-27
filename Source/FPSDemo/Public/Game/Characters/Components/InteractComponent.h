// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Game/Items/Pickup/PickupItem.h"
#include "Components/ActorComponent.h"
#include "InteractComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FShowInteractMessage, const FText&);
DECLARE_MULTICAST_DELEGATE(FHideInteractMessage)

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FPSDEMO_API UInteractComponent : public UActorComponent
{
	GENERATED_BODY()
private:
	UPROPERTY()
	TWeakObjectPtr<AActor> FocusedActor;
public:	
	// Sets default values for this component's properties
	UInteractComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UFUNCTION(Server, Reliable)
	void ServerTryPickup(APickupItem* Item);
	void HandlePickup_Internal(APickupItem* Item);
	void SetFocus(AActor* NewActor);
	void TraceFocus();
	void EndFocus(AActor* Actor);
	void StartFocus(AActor* Actor);
public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	void TryPickup();	
	FShowInteractMessage ShowInteractMessage;
	FHideInteractMessage HideInteractMessage;

	static constexpr float FocusRange = 300.0f;
};
