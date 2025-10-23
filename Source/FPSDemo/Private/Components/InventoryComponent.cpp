// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/InventoryComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameConstants.h"
#include "Items/ItemIds.h"
#include "Weapons/WeaponData.h"

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
	GMR = GetWorld()->GetGameInstance()->GetSubsystem<UGameManager>();
	Super::BeginPlay();
	GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
		{
			InitState();
		});

	// Default slot melee weapon
	if (GetOwnerRole() == ROLE_Authority)
	{
		UE_LOG(LogTemp, Warning, TEXT("InventoryComponent: Adding default melee weapon to inventory"));
		FInventoryItem MeleeItem;
		MeleeItem.ItemId = EItemId::MELEE_KNIFE_BASIC;
		MeleeItem.Count = 1;
		MeleeItem.InventoryId = IdCounter++;
		MeleeItem.AmmoInMag = 0;
		Items.Add(MeleeItem);

		FInventoryItem Item2;
		Item2.ItemId = EItemId::GRENADE_FRAG_BASIC;
		Item2.Count = 1;
		Item2.InventoryId = IdCounter++;
		Item2.AmmoInMag = 0;
		Items.Add(Item2);
	}
}

void UInventoryComponent::InitState() {
	if (GetOwnerRole() == ROLE_Authority)
	{
		
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("InventoryComponent: Client InitState called, triggering OnRep_Items"));
		OnRep_Items();
	}
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

void UInventoryComponent::RemoveItemByInventoryId(int32 InventoryId)
{
	for (int32 i = 0; i < Items.Num(); ++i)
	{
		if (Items[i].InventoryId == InventoryId)
		{
			Items.RemoveAt(i);
			UE_LOG(LogTemp, Log, TEXT("Item with InventoryId %d removed from inventory."), InventoryId);
			return;
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("Item with InventoryId %d not found in inventory."), InventoryId);
}

int32 UInventoryComponent::GetInventoryIdBySlot(int32 Slot){
	if (SlotMap.Contains(Slot)) {
		return SlotMap[Slot];
	}

	return FGameConstants::INVENTORY_ID_NONE; // Not found
}

void UInventoryComponent::OnRep_Items()
{
	UE_LOG(LogTemp, Warning, TEXT("Inventory updated, new count: %d"), Items.Num());
	// Refresh UI or rebuild slot map here
}

int32 UInventoryComponent::GetFirstInventoryIdByType(EWeaponTypes ItemType)
{
	for (const FInventoryItem& Item : Items)
	{
		if (GMR)
		{
			const UItemData* ItemData = GMR->GetItemDataById(Item.ItemId);
			if (ItemData && ItemData->IsA(UWeaponData::StaticClass()))
			{
				const UWeaponData* WeaponData = Cast<UWeaponData>(ItemData);
				if (WeaponData && WeaponData->WeaponType == ItemType)
				{
					return Item.InventoryId;
				}
			}
		}
	}
	return FGameConstants::INVENTORY_ID_NONE; // Not found
}

TArray<FInventoryItem> UInventoryComponent::GetItems() const {
	return Items;
}

bool UInventoryComponent::CheckExistItem(int InventoryId) {
	for (const FInventoryItem& Item : Items)
	{
		if (Item.InventoryId == InventoryId) {
			return true;
		}
	}
	return false;
}


int32 UInventoryComponent::GetMeleeId() {
	for (const FInventoryItem& Item : Items)
	{
		if (GMR)
		{
			const UItemData* ItemData = GMR->GetItemDataById(Item.ItemId);
			if (ItemData && ItemData->IsA(UWeaponData::StaticClass()))
			{
				const UWeaponData* WeaponData = Cast<UWeaponData>(ItemData);
				if (WeaponData && WeaponData->WeaponType == EWeaponTypes::Melee)
				{
					return Item.InventoryId;
				}
			}
		}
	}
	return FGameConstants::INVENTORY_ID_NONE; // Not found
}