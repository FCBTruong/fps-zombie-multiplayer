// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/PickupComponent.h"
#include "Components/InventoryComponent.h"
#include "Weapons/WeaponDataManager.h"
#include "Pickup/PickupItem.h"
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

	// ...
}

void UPickupComponent::PickupItem(APickupItem* Item)
{
	if (!Item) return;
	FPickupData PickupData = Item->GetData();

	// Add to inventory logic here
	FString ItemName = StaticEnum<EItemId>()->GetNameStringByValue(static_cast<int64>(PickupData.ItemId));
	UE_LOG(LogTemp, Warning, TEXT("Picked up item with ID: %s"), *ItemName);

	UInventoryComponent* Inventory = GetOwner()->FindComponentByClass<UInventoryComponent>();
	if (Inventory)
	{
		// Get the item data from your items manager
		UWeaponDataManager* WeaponDataMgr = GetOwner()->GetGameInstance()->GetSubsystem<UWeaponDataManager>();
		
		UItemData* ItemData = WeaponDataMgr->GetWeaponById(PickupData.ItemId);
		if (ItemData)
		{
			Inventory->AddItem(*ItemData); // Assuming you have an AddItem method in your inventory component
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("ItemData not found for ItemId: %s"), *ItemName);
		}
	}
	Item->Destroy();
}