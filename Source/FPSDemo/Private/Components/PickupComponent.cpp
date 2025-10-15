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

	GMR = GetWorld()->GetGameInstance()->GetSubsystem<UGameManager>();
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

// This function runs on the server
void UPickupComponent::HandlePickupItem(int32 ItemOnMapId) {
	UE_LOG(LogTemp, Log, TEXT("HandlePickupItem called with ItemOnMapId: %d"), ItemOnMapId);
	UInventoryComponent* Inventory = GetOwner()->FindComponentByClass<UInventoryComponent>();
	if (!Inventory) {
		UE_LOG(LogTemp, Warning, TEXT("Inventory component not found on actor: %s"), *GetOwner()->GetName());
		return;
	}

	if (!GMR)
	{
		UE_LOG(LogTemp, Warning, TEXT("GameManager subsystem not found"));
		return;
	}

	FPickupData PickupData = GMR->GetDataPickupItem(ItemOnMapId);
	if (PickupData.Id == -1) {
		UE_LOG(LogTemp, Warning, TEXT("Pickup data not found for ItemOnMapId: %d"), ItemOnMapId);
		return;
	}

	// Check if player nearby or not
	FVector ItemLocation = PickupData.Location;
	FVector PlayerLocation = GetOwner()->GetActorLocation();
	float Distance = FVector::Dist2D(ItemLocation, PlayerLocation);

	if (Distance > 500.f) {
		UE_LOG(LogTemp, Warning, TEXT("Player is too far to pick up the item. Distance: %f"), Distance);
		return;
	}
	
	// Get the item data from your items manager
	UItemData* ItemData = GMR->GetItemDataById(PickupData.ItemId);
	int32 NewInventoryId = Inventory->AddItem(*ItemData);

	UWeaponComponent* WeaponComp = GetOwner()->FindComponentByClass<UWeaponComponent>();
	WeaponComp->OnNewItemPickup(NewInventoryId);

	MulticastPickupItem(ItemOnMapId);
	GMR->FindAndDestroyItem(ItemOnMapId);
}

// This function runs on all clients
void UPickupComponent::MulticastPickupItem_Implementation(int32 ItemOnMapId)
{
	FPickupData PickupData = GMR->GetDataPickupItem(ItemOnMapId);
	GMR->FindAndDestroyItem(ItemOnMapId);
}