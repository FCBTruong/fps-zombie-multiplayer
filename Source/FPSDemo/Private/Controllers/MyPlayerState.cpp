// Fill out your copyright notice in the Description page of Project Settings.


#include "Controllers/MyPlayerState.h"
#include "Characters/BaseCharacter.h"


AMyPlayerState::AMyPlayerState()
{
	SetNetUpdateFrequency(100.f);
	SetMinNetUpdateFrequency(30.f);
}

void AMyPlayerState::ProcessBuy(const UWeaponData* Item)
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
			UWeaponComponent* WepComp = MyChar->GetWeaponComponent();
			if (!WepComp) {
				UE_LOG(LogTemp, Warning, TEXT("ServerBuyItem: WeaponComponent is null"));
				return;
			}
			FPickupData PickupData;
			PickupData.ItemId = Item->Id;

			// for weapons, set initial ammo
			
			PickupData.AmmoInClip = Item->MaxAmmoInClip;
			PickupData.AmmoReserve = Item->AmmoBonusShop;
			WepComp->AddNewWeapon(PickupData);
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
	DOREPLIFETIME(AMyPlayerState, TeamID);
	DOREPLIFETIME(AMyPlayerState, bIsAlive);
	DOREPLIFETIME(AMyPlayerState, Kills);
	DOREPLIFETIME(AMyPlayerState, Deaths);
	DOREPLIFETIME(AMyPlayerState, Assists);
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

bool AMyPlayerState::CanBuyThisItem(const UItemData* Item) const
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
	return Money >= Item->Price;
}

void AMyPlayerState::OnRep_TeamId()
{
	UE_LOG(LogTemp, Warning, TEXT("TeamId updated: %s"), *TeamID.ToString());
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

	TryBuySlot(EWeaponSubTypes::Rifle);
}

void AMyPlayerState::TryBuySlot(EWeaponSubTypes Type)
{
	TArray<UWeaponData*> AllWeapons =
		UGameManager::Get(GetWorld())
		->GetWeaponDataManager()
		->GetAllWeapons();

	// filter by type
	TArray<UWeaponData*> FilteredWeapons;
	for (UWeaponData* Wep : AllWeapons)
	{
		if (Wep->WeaponSubType == Type)
		{
			FilteredWeapons.Add(Wep);
		}
	}

	// sort by price ascending
	/*FilteredWeapons.Sort([](const UWeaponData& A, const UWeaponData& B) {
		return A.Price > B.Price;
		});*/

	if (FilteredWeapons.Num() == 0)
	{
		return;
	}

	int32 RandomIndex = FMath::RandRange(0, FilteredWeapons.Num() - 1);
	UWeaponData* RandomWeapon = FilteredWeapons[RandomIndex];

	if (CanBuyThisItem(RandomWeapon))
	{
		ProcessBuy(RandomWeapon);
	}
}