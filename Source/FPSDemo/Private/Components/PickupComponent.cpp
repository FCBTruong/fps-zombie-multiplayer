// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/PickupComponent.h"
#include "Components/InventoryComponent.h"
#include "Pickup/PickupItem.h"
#include "Game/GameManager.h"
#include "Characters/BaseCharacter.h"
#include "Game/SpikeMode.h"
#include "Components/EquipComponent.h"
#include "Game/GlobalDataAsset.h"
#include <Kismet/GameplayStatics.h>

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

void UPickupComponent::PickupItem(APickupItem* PickupItem, bool AutoEquip)
{
	if (!IsEnabled()) {
		return;
	}
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
	
	auto PickupData = PickupItem->GetPickupData();
	// special case, for spike, need to check team
	if (PickupData.ItemId == EItemId::SPIKE) {
		// check team
		ETeamId Team = OwnerCharacter->GetTeamId();
		if (Team != ETeamId::Attacker) {
			return;
		}
	}

	// Check if overlap
    bool Added = InventoryComp->AddItemFromPickup(PickupData);
	UE_LOG(LogTemp, Warning, TEXT("PickupItem: Added = %s"), Added ? TEXT("true") : TEXT("false"));
	if (Added) {
		GMR->FindAndDestroyItemNode(PickupData.Id);

		// check if is spike, need to tell game mode
		if (PickupData.ItemId == EItemId::SPIKE) {
			// tell spike mode
			if (ASpikeMode* SpikeGM = Cast<ASpikeMode>(GetWorld()->GetAuthGameMode()))
			{
				SpikeGM->NotifySpikePickedUp(OwnerCharacter);
			}
		}

		if (AutoEquip)
		{
			UE_LOG(LogTemp, Warning, TEXT("AutoEquip is true, equipping item"));
			UEquipComponent* EquipComp = OwnerCharacter->GetEquipComponent();
			if (EquipComp)
			{
				EquipComp->RequestSelectActiveItem(PickupItem->GetPickupData().ItemId);
			}
		}

		// notify client
		APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController());
		if (PC)
		{
			// only for player, not for AI
			ClientNotifyItemPickup(PickupItem->GetPickupData().ItemId);
		}
	}
}

void UPickupComponent::ClientNotifyItemPickup_Implementation(
	EItemId ItemId)
{
	UE_LOG(LogTemp, Warning, TEXT("ClientNotifyItemPickup called for item id: %d"), static_cast<uint8>(ItemId));
	ABaseCharacter* OwnerCharacter = Cast<ABaseCharacter>(GetOwner());
	if (!OwnerCharacter)
	{
		return;
	}

	// play sound
	UGameManager* GM = UGameManager::Get(GetWorld());
	UGlobalDataAsset* GlobalData = GM->GlobalData;
	UGameplayStatics::PlaySound2D(
		GetWorld(),
		GM->GlobalData->PickupSound
	);

	// broadcast to UI
	OnNewItemPickup.Broadcast(ItemId);
}
