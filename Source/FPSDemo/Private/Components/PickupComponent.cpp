// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/PickupComponent.h"
#include "Components/InventoryComponent.h"
#include "Weapons/WeaponDataManager.h"
#include "Components/WeaponComponent.h"
#include "Pickup/PickupItem.h"
#include "Game/GameManager.h"
#include "Characters/BaseCharacter.h"

// Sets default values for this component's properties
UPickupComponent::UPickupComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UPickupComponent::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Warning, TEXT("PickupComponent::BeginPlay called"));
	GMR = UGameManager::Get(GetWorld());
}


// Called every frame
void UPickupComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

void UPickupComponent::PickupItem(APickupItem* PickupItem)
{
	ABaseCharacter* OwnerCharacter = Cast<ABaseCharacter>(GetOwner());
	if (!OwnerCharacter)
	{
		return;
	}

	if (!OwnerCharacter->IsAlive())
	{
		return;
	}

	if (!PickupItem) {
		UE_LOG(LogTemp, Warning, TEXT("PickupItem called with null Item"));
		return;
	}
	if (!GMR)
	{
		UE_LOG(LogTemp, Warning, TEXT("GameManager subsystem not found"));
		return;
	}

	UInventoryComponent* InventoryComp = OwnerCharacter->GetInventoryComponent();

	if (!InventoryComp) {
		UE_LOG(LogTemp, Warning, TEXT("InventoryComponent not found on character"));
		return;
	}
	
	// Check if overlap
    bool Added = InventoryComp->AddItemFromPickup(PickupItem->GetPickupData());
	UE_LOG(LogTemp, Warning, TEXT("PickupItem: Added = %s"), Added ? TEXT("true") : TEXT("false"));
	if (Added) {
		GMR->FindAndDestroyItemNode(PickupItem->GetPickupData().Id);

		// notify client
		ClientNotifyItemPickup(PickupItem->GetPickupData().ItemId);
	}
}

void UPickupComponent::ClientNotifyItemPickup_Implementation(
	EItemId ItemId)
{
	ABaseCharacter* OwnerCharacter = Cast<ABaseCharacter>(GetOwner());
	if (!OwnerCharacter)
	{
		return;
	}
	if (!OwnerCharacter->IsLocallyControlled())
	{
		return;
	}
	// broadcast to UI
	OnNewItemPickup.Broadcast(ItemId);
}
