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
	// Implement your buy logic here
	UE_LOG(LogTemp, Warning, TEXT("Processing buy for item: %s"), *Item->GetName());

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
		double Time = FPlatformTime::Seconds();
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
	// This function is called on clients when Money is updated on the server
	UE_LOG(LogTemp, Warning, TEXT("Money updated: %d"), Money);

	OnUpdateMoney.Broadcast();
}

void AMyPlayerState::OnRep_BoughtItems()
{
	// This function is called on clients when BoughtItems is updated on the server
	UE_LOG(LogTemp, Warning, TEXT("BoughtItems updated. Total items bought: %d"), BoughtItems.Num());
	double Time = FPlatformTime::Seconds();
	UE_LOG(LogTemp, Warning, TEXT("DebugTime - OnRep_BoughtItems: %.3f"), Time);
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
	// Print current owned weapons
	UE_LOG(LogTemp, Log, TEXT("PlayerState [%s] this=%p OwnedWeapons:"),
		*GetName(), this);

	for (EItemId Item : OwnedWeapons)
	{
		UE_LOG(LogTemp, Log, TEXT("  - %s"),
			*UEnum::GetValueAsString(Item));
	}

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
	PlayerSlot = Slot;

	// set crosshair code
	if (PlayerSlot) {
		SetCrosshairCode(PlayerSlot->GetCrosshairCode());
	}
}

void AMyPlayerState::SetCrosshairCode(const FString& InCrosshairCode)
{
	CrosshairCode = InCrosshairCode;
}