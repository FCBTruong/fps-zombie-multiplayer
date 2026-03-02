// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/Characters/Components/PickupComponent.h"
#include "Game/Characters/Components/InventoryComponent.h"
#include "Game/Items/Pickup/PickupItem.h"
#include "Game/GameManager.h"
#include "Game/Characters/BaseCharacter.h"
#include "Game/Modes/Spike/SpikeMode.h"
#include "Game/Characters/Components/EquipComponent.h"
#include "Shared/Data/GlobalDataAsset.h"
#include <Kismet/GameplayStatics.h>
#include "Game/Subsystems/ActorManager.h"

// Sets default values for this component's properties
UPickupComponent::UPickupComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

// Server called
void UPickupComponent::PickupItem(APickupItem* PickupItem, bool AutoEquip)
{
	if (!GetOwner()->HasAuthority())
	{
		return;
	}

	if (!IsEnabled()) {
		return;
	}
	ABaseCharacter* OwnerCharacter = Cast<ABaseCharacter>(GetOwner());
	if (!IsValid(OwnerCharacter))
	{
		return;
	}

	if (!OwnerCharacter->IsAlive())
	{
		return;
	}

	if (!IsValid(PickupItem)) {
		UE_LOG(LogTemp, Warning, TEXT("UPickupComponent: PickupItem is not valid"));
		return;
	}

	AActorManager* ActorMgr = AActorManager::Get(GetWorld());
	if (!ActorMgr) {
		UE_LOG(LogTemp, Warning, TEXT("UPickupComponent: ActorManager not found in PickupItem"));
		return;
	}

	UInventoryComponent* InventoryComp = OwnerCharacter->GetInventoryComponent();
	if (!InventoryComp) {
		UE_LOG(LogTemp, Warning, TEXT("UPickupComponent: InventoryComponent not found on character"));
		return;
	}
	
	auto PickupData = PickupItem->GetData();
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
	if (Added) {
		ActorMgr->FindAndDestroyItemNode(PickupData.Id);

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
			UEquipComponent* EquipComp = OwnerCharacter->GetEquipComponent();
			if (EquipComp)
			{
				EquipComp->RequestSelectActiveItem(PickupData.ItemId);
			}
		}

		// notify client
		APlayerController* PC = Cast<APlayerController>(OwnerCharacter->GetController());
		if (PC)
		{
			// only for player, not for AI
			ClientNotifyItemPickup(PickupItem->GetData().ItemId);
		}
	}
}

void UPickupComponent::ClientNotifyItemPickup_Implementation(
	EItemId ItemId)
{
	// play sound
	UGameManager* GM = UGameManager::Get(GetWorld());
	if (!GM || !GM->GlobalData) {
		UE_LOG(LogTemp, Warning, TEXT("GameManager or GlobalData not found in ClientNotifyItemPickup"));
		return;
	}
	UGlobalDataAsset* GlobalData = GM->GlobalData;
	if (GlobalData->PickupSound) {
		UGameplayStatics::PlaySound2D(
			GetWorld(),
			GM->GlobalData->PickupSound
		);
	}

	// broadcast to UI
	OnNewItemPickup.Broadcast(ItemId);
}
