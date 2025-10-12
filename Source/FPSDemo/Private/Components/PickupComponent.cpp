// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/PickupComponent.h"
#include "Components/InventoryComponent.h"
#include "Weapons/WeaponDataManager.h"
#include "Components/WeaponComponent.h"
#include "Pickup/PickupItem.h"
#include <Game/GameManager.h>

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

	// ...
	
}


// Called every frame
void UPickupComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

void UPickupComponent::PickupItem(APickupItem* Item)
{
	ServerPickupItem(Item->GetData().Id);
}

void UPickupComponent::ServerPickupItem_Implementation(int32 ItemOnMapId)
{
	HandlePickupItem(ItemOnMapId);
}

void UPickupComponent::HandlePickupItem(int32 ItemOnMapId) {
	UInventoryComponent* Inventory = GetOwner()->FindComponentByClass<UInventoryComponent>();
	if (!Inventory) {
		UE_LOG(LogTemp, Warning, TEXT("Inventory component not found on actor: %s"), *GetOwner()->GetName());
		return;
	}
	UGameManager* GMR = GetWorld()->GetGameInstance()->GetSubsystem<UGameManager>();
	if (!GMR)
	{
		return;
	}

	FPickupData PickupData = GMR->GetDataPickupItem(ItemOnMapId);
	if (PickupData.Id == -1) {
		UE_LOG(LogTemp, Warning, TEXT("Pickup data not found for ItemOnMapId: %d"), ItemOnMapId);
		return;
	}

	// Check if player nearby or not
	FVector ItemLocation = PickupData.Location;

	bool IsPickedUp = false;
	
	// Get the item data from your items manager
	UWeaponDataManager* WeaponDataMgr = GetOwner()->GetGameInstance()->GetSubsystem<UWeaponDataManager>();

	UItemData* ItemData = WeaponDataMgr->GetWeaponById(PickupData.ItemId);
	if (ItemData)
	{
		Inventory->AddItem(*ItemData); // Assuming you have an AddItem method in your inventory component

		UWeaponComponent* WeaponComp = GetOwner()->FindComponentByClass<UWeaponComponent>();
		WeaponComp->OnNewItemPickup(PickupData.ItemId);
		IsPickedUp = true;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ItemData not found for ItemId"));
	}
	
	if (IsPickedUp) {
		GMR->FindAndDestroyItem(ItemOnMapId);
	}
}