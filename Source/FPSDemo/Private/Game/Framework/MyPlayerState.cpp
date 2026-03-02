// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/Framework/MyPlayerState.h"
#include "Game/Characters/BaseCharacter.h"
#include "Game/Items/Pickup/PickupData.h"
#include "Game/Characters/Components/InventoryComponent.h"
#include "Shared/Data/Items/ItemConfig.h"
#include "Shared/Data/Items/FirearmConfig.h"
#include "Net/UnrealNetwork.h"
#include "Game/Framework/ShooterGameState.h"
#include "Game/Framework/PlayerSlot.h"

AMyPlayerState::AMyPlayerState()
{
	SetNetUpdateFrequency(100.f);
	SetMinNetUpdateFrequency(30.f);
	bUseCustomPlayerNames = true;
}

void AMyPlayerState::ProcessBuy(const UItemConfig* Item)
{
	if (!Item)
	{
		UE_LOG(LogTemp, Warning, TEXT("ProcessBuy called with null Item"));
		return;
	}

	// Check if item has already been bought
	if (BoughtItems.Contains(Item->Id))
	{
		UE_LOG(LogTemp, Warning, TEXT("Item %s has already been bought"), *Item->GetName());
		return;
	}

	if (Money >= Item->Price)
	{
		// check whether the player can carry the item, etc.
		
		// add item to inventory logic would go here
		Money -= Item->Price;
		BoughtItems.Add(Item->Id);

		ABaseCharacter* MyChar = Cast<ABaseCharacter>(GetPawn());
		if (MyChar)
		{
			UInventoryComponent* InvComp = MyChar->GetInventoryComponent();
			FPickupData PickupData;
			PickupData.ItemId = Item->Id;

			// for weapons, set initial ammo
			if (Item->GetItemType() == EItemType::Firearm) {
				const UFirearmConfig* FirearmItem = Cast<UFirearmConfig>(Item);
				PickupData.AmmoInClip = FirearmItem->MaxAmmoInClip;
				PickupData.AmmoReserve = FirearmItem->AmmoBonus;
			}
			InvComp->AddItemFromShop(PickupData);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Not enough money to buy item: %s"), *Item->GetName());
	}
}

void AMyPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMyPlayerState, Money);
	DOREPLIFETIME(AMyPlayerState, BoughtItems);
}

void AMyPlayerState::OnRep_Money()
{
	OnUpdateMoney.Broadcast();
}

void AMyPlayerState::OnRep_BoughtItems()
{
	OnUpdateBoughtItems.Broadcast();
}

bool AMyPlayerState::CanBuyThisItem(const UItemConfig* Item) const
{
	if (!Item)
	{
		return false;
	}
	// Check if item has already been bought
	if (BoughtItems.Contains(Item->Id))
	{
		return false;
	}

	if (Item->GetItemType() == EItemType::Firearm)
	{
		// Check if player already owns this weapon
		if (OwnedWeapons.Contains(Item->Id))
		{
			return false;
		}
	}
	return Money >= Item->Price;
}

void AMyPlayerState::AutoBuy() {
	TryBuySlot();
}

void AMyPlayerState::TryBuySlot()
{
	
}

ETeamId AMyPlayerState::GetTeamId() const
{
	if (PlayerSlot)
	{
		return PlayerSlot->GetTeamId();
	}
	return ETeamId::None;
}

void AMyPlayerState::AddKill() {
	if (PlayerSlot) {
		PlayerSlot->AddKill();
	}
}

void AMyPlayerState::AddDeath() {
	if (PlayerSlot) {
		PlayerSlot->AddDeath();
	}
}

void AMyPlayerState::AddAssist() {
	if (PlayerSlot) {
		PlayerSlot->AddAssist();
	}
}

int AMyPlayerState::GetKills() const {
	if (PlayerSlot) {
		return PlayerSlot->GetKills();
	}
	return 0;
}

int AMyPlayerState::GetDeaths() const {
	if (PlayerSlot) {
		return PlayerSlot->GetDeaths();
	}
	return 0;
}

int AMyPlayerState::GetAssists() const {
	if (PlayerSlot) {
		return PlayerSlot->GetAssists();
	}
	return 0;
}

int AMyPlayerState::GetBackendUserId() const
{
	if (PlayerSlot)
	{
		return PlayerSlot->GetBackendUserId();
	}
	return 0;
}

void AMyPlayerState::SetTeamId(ETeamId NewTeamId)
{
	if (PlayerSlot)
	{
		PlayerSlot->SetTeamId(NewTeamId);
	}
}

FString AMyPlayerState::GetPlayerNameCustom() const
{
	return PlayerSlot ? PlayerSlot->GetPlayerName() : "";
}

void AMyPlayerState::SetPlayerSlot(APlayerSlot* Slot)
{
	// unbind from old slot's events
	if (PlayerSlot)
	{
		PlayerSlot->OnUpdateTeamId.RemoveAll(this);
	}

	PlayerSlot = Slot;
	// set crosshair code
	if (PlayerSlot) {
		SetCrosshairCode(PlayerSlot->GetCrosshairCode());
	}

	OnRep_PlayerSlot();
}

void AMyPlayerState::SetCrosshairCode(const FString& InCrosshairCode)
{
	CrosshairCode = InCrosshairCode;
}

void AMyPlayerState::OnRep_PlayerSlot()
{
	if (PlayerSlot)
	{
		PlayerSlot->OnUpdateTeamId.RemoveAll(this);
		PlayerSlot->OnUpdateTeamId.AddUObject(this, &AMyPlayerState::HandlePlayerSlotTeamIdUpdated);
	}
}

void AMyPlayerState::HandlePlayerSlotTeamIdUpdated(ETeamId NewTeamId)
{
	OnUpdateTeamId.Broadcast(NewTeamId);
}