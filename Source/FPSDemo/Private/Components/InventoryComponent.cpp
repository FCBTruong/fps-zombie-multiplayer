// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/InventoryComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UInventoryComponent::UInventoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	SetIsReplicated(true);
}


// Called when the game starts
void UInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UInventoryComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UInventoryComponent, Items);
}

// This function only called on server side
int32 UInventoryComponent::AddItem(const UItemData& ItemData)
{
	// Implement your logic to add the item to the inventory
	FString ItemName = ItemData.DisplayName.ToString();
	UE_LOG(LogTemp, Log, TEXT("Item added to inventory: %s"), *ItemName);

	FInventoryItem NewItem;
	NewItem.ItemId = ItemData.Id;
	NewItem.Count = 1; // Default count
	NewItem.InventoryId = IdCounter++;
	NewItem.AmmoInMag = 0; // Default ammo in mag

	Items.Add(NewItem);
	return NewItem.InventoryId;
}

FInventoryItem* UInventoryComponent::GetItemByInventoryId(int32 InventoryId)
{
	for (FInventoryItem& Item : Items)
	{
		if (Item.InventoryId == InventoryId)
		{
			return &Item;
		}
	}
	return nullptr;
}