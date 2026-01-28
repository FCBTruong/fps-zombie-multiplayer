// WeaponInventoryComponent.cpp

#include "Components/InventoryComponent.h"
#include "Game/GameManager.h"
#include "Weapons/WeaponState.h"       
#include "Pickup/PickupData.h"
#include "Game/ItemsManager.h"
#include "Items/ItemConfig.h"
#include "Items/FirearmConfig.h"
#include "Net/UnrealNetwork.h"
#include "Pickup/PickupItem.h"
#include "Characters/BaseCharacter.h"
#include "Items/ArmorConfig.h"

UInventoryComponent::UInventoryComponent()
{
    SetIsReplicatedByDefault(true);
}

void UInventoryComponent::BeginPlay()
{
    Super::BeginPlay();
	UE_LOG(LogTemp, Log, TEXT("UInventoryComponent::BeginPlay called"));
    CachedGM = UGameManager::Get(GetWorld());
}

void UInventoryComponent::Test()
{
    // Test function to be removed later
    MeleeState.ItemId = EItemId::MELEE_KNIFE_BASIC;
    PistolState.ItemId = EItemId::PISTOL_PL_14;
	RifleState.ItemId = EItemId::RIFLE_AK_47;

	const UItemConfig* ItemPistol = UItemsManager::Get(GetWorld())->GetItemById(EItemId::PISTOL_PL_14);
	const UFirearmConfig* PistolData = Cast<UFirearmConfig>(ItemPistol);
    PistolState.AmmoInClip = PistolData ? PistolData->MaxAmmoInClip : 0;
    PistolState.AmmoReserve = PistolData ? PistolData->MaxAmmoInClip * 2 : 0;
	PistolState.MaxAmmoInClip = PistolData ? PistolData->MaxAmmoInClip : 0;

	const UItemConfig* ItemRifle = UItemsManager::Get(GetWorld())->GetItemById(EItemId::RIFLE_AK_47);
	const UFirearmConfig* RifleData = Cast<UFirearmConfig>(ItemRifle);
	RifleState.AmmoInClip = 2;
    RifleState.AmmoReserve = RifleData ? RifleData->MaxAmmoInClip * 3 : 0;
	RifleState.MaxAmmoInClip = RifleData ? RifleData->MaxAmmoInClip : 0;

    Throwables.Add(EItemId::GRENADE_FRAG_BASIC);
    Throwables.Add(EItemId::GRENADE_SMOKE);
    Throwables.Add(EItemId::GRENADE_INCENDIARY);
    Throwables.Add(EItemId::GRENADE_STUN);
             
    // log size throwables
	UE_LOG(LogTemp, Log, TEXT("InventoryComponent Test: Throwables count = %d"), Throwables.Num());
}


void UInventoryComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UInventoryComponent, RifleState);
    DOREPLIFETIME(UInventoryComponent, PistolState);
    DOREPLIFETIME(UInventoryComponent, MeleeState);
    DOREPLIFETIME(UInventoryComponent, Throwables);
    DOREPLIFETIME(UInventoryComponent, ArmorState);
    DOREPLIFETIME(UInventoryComponent, bHasSpike);
}

const UItemConfig* UInventoryComponent::GetItemConfig(EItemId ItemId)
{
    if (ItemId == EItemId::NONE) return nullptr;

	return UItemsManager::Get(GetWorld())->GetItemById(ItemId);
}

void UInventoryComponent::SortThrowables()
{
    Throwables.Sort([](const EItemId& A, const EItemId& B)
        {
            return static_cast<int32>(A) < static_cast<int32>(B);
        });
}

FWeaponState* UInventoryComponent::GetWeaponStateByItemId(EItemId ItemId)
{
    if (RifleState.ItemId == ItemId) return &RifleState;
    if (PistolState.ItemId == ItemId) return &PistolState;
    if (MeleeState.ItemId == ItemId) return &MeleeState;
    return nullptr;
}

const FWeaponState* UInventoryComponent::GetWeaponStateByItemId(EItemId ItemId) const
{
    if (RifleState.ItemId == ItemId) return &RifleState;
    if (PistolState.ItemId == ItemId) return &PistolState;
    if (MeleeState.ItemId == ItemId) return &MeleeState;
    return nullptr;
}

bool UInventoryComponent::CanDrop(EItemId ItemId)
{
    const UItemConfig* Data = GetItemConfig(ItemId);
    return Data ? Data->bIsDroppable : false;
}

bool UInventoryComponent::AddItemInternal(const FPickupData& PickupData)
{
    if (!GetOwner() || !GetOwner()->HasAuthority())
        return false;

    const EItemId ItemId = PickupData.ItemId;
    const UItemConfig* Data = GetItemConfig(ItemId);
    if (!Data) {
		UE_LOG(LogTemp, Warning, TEXT("UInventoryComponent::AddItemInternal: Invalid ItemId %d"), static_cast<int32>(ItemId));
        return false;
    }

    EItemType ItemType = Data->GetItemType();
    switch (ItemType)
    {
        case EItemType::Firearm:
        {
			const UFirearmConfig* FirearmData = Cast<UFirearmConfig>(Data);
            
            if (FirearmData->FirearmType == EFirearmType::Rifle)
            {
                if (RifleState.ItemId != EItemId::NONE) {
					DropItem(RifleState.ItemId);
                    //return false;
                }
                RifleState.ItemId = ItemId;
                RifleState.AmmoInClip = PickupData.AmmoInClip;
                RifleState.AmmoReserve = PickupData.AmmoReserve;
                RifleState.MaxAmmoInClip = FirearmData->MaxAmmoInClip;
                OnRep_RifleState();
                return true;
            }
            else 
            {
                if (PistolState.ItemId != EItemId::NONE) return false;
                PistolState.ItemId = ItemId;
                PistolState.AmmoInClip = PickupData.AmmoInClip;
                PistolState.AmmoReserve = PickupData.AmmoReserve;
                PistolState.MaxAmmoInClip = FirearmData->MaxAmmoInClip;
                OnRep_PistolState();
                return true;
            }
        }
        case EItemType::Melee:
        {
            if (MeleeState.ItemId != EItemId::NONE) return false;
            MeleeState.ItemId = ItemId;
            // Melee ammo typically unused
            OnRep_MeleeState();
            return true;
        }
        case EItemType::Throwable:
        {
            Throwables.Add(ItemId);
            SortThrowables();
            OnRep_Throwables();
            return true;
        }
        case EItemType::Armor:
        {
            ApplyArmorItem(ItemId);
            return true;
        }

        default:
            if (ItemId == EItemId::SPIKE) {
                SetHasSpike(true);
                return true;
			}
            return false;
    }
}

bool UInventoryComponent::RemoveThrowable(EItemId ItemId)
{
    if (!GetOwner() || !GetOwner()->HasAuthority())
        return false;

    const int32 Removed = Throwables.Remove(ItemId);
    if (Removed > 0)
    {
        OnRep_Throwables();
        return true;
    }
    return false;
}

void UInventoryComponent::SetHasSpike(bool bNewHasSpike)
{
    if (!GetOwner() || !GetOwner()->HasAuthority())
        return;

    if (bHasSpike == bNewHasSpike)
        return;

    bHasSpike = bNewHasSpike;
    OnRep_HasSpike();
}

void UInventoryComponent::ApplyArmorItem(EItemId ArmorItemId)
{
    if (!GetOwner() || !GetOwner()->HasAuthority())
    {
        return;
    }

	const UItemConfig* Data = GetItemConfig(ArmorItemId);
	const UArmorConfig* ArmorData = Cast<UArmorConfig>(Data);

    if (!ArmorData)
    {
        return;
    }

	ArmorState.ArmorEfficiency = ArmorData->ArmorEfficiency;
	ArmorState.ArmorRatio = ArmorData->ArmorRatio;
	ArmorState.ArmorMaxPoints = ArmorData->ArmorMaxPoints;
	ArmorState.ArmorPoints = ArmorData->ArmorMaxPoints;

    OnRep_ArmorState();
}

bool UInventoryComponent::ConsumeAmmo(EItemId WeaponId, int32 Amount)
{
    if (!GetOwner() || !GetOwner()->HasAuthority())
        return false;

    if (Amount <= 0) return true;

    FWeaponState* WS = GetWeaponStateByItemId(WeaponId);
    if (!WS) return false;

    WS->AmmoInClip = FMath::Max(0, WS->AmmoInClip - Amount);
    
    // On Rep manually
    if (WS == &RifleState)
        OnRep_RifleState();
    else if (WS == &PistolState)
		OnRep_PistolState();

    return true;
}

int32 UInventoryComponent::ReloadAmmo(EItemId WeaponId, int32 MaxAmmoInClip)
{
    if (!GetOwner() || !GetOwner()->HasAuthority())
        return 0;

    FWeaponState* WS = GetWeaponStateByItemId(WeaponId);
    if (!WS) return 0;

    const int32 Needed = MaxAmmoInClip - WS->AmmoInClip;
    if (Needed <= 0) return 0;

    const int32 ToLoad = FMath::Min(Needed, WS->AmmoReserve);
    WS->AmmoReserve = FMath::Max(0, WS->AmmoReserve - ToLoad);
    WS->AmmoInClip += ToLoad;
    return ToLoad;
}

// ===== OnRep =====

void UInventoryComponent::OnRep_RifleState()
{
    OnRifleChanged.Broadcast(RifleState.ItemId);
    OnInventoryChanged.Broadcast();
	OnAmmoDataChanged.Broadcast(RifleState.ItemId, RifleState.AmmoInClip, RifleState.AmmoReserve);
}

void UInventoryComponent::OnRep_PistolState()
{
    OnPistolChanged.Broadcast(PistolState.ItemId);
    OnInventoryChanged.Broadcast();
	OnAmmoDataChanged.Broadcast(PistolState.ItemId, PistolState.AmmoInClip, PistolState.AmmoReserve);
}

void UInventoryComponent::OnRep_MeleeState()
{
    OnMeleeChanged.Broadcast(MeleeState.ItemId);
    OnInventoryChanged.Broadcast();
}

void UInventoryComponent::OnRep_Throwables()
{
    OnThrowablesChanged.Broadcast(Throwables);
    OnInventoryChanged.Broadcast();
}

void UInventoryComponent::OnRep_ArmorState()
{
    OnArmorChanged.Broadcast(ArmorState.ArmorPoints, ArmorState.ArmorMaxPoints);
    OnInventoryChanged.Broadcast();
}

void UInventoryComponent::OnRep_HasSpike()
{
    OnSpikeChanged.Broadcast(bHasSpike);
    OnInventoryChanged.Broadcast();
}

void UInventoryComponent::ReloadWeapon(EItemId Id) {
    FWeaponState* State = this->GetWeaponStateByItemId(Id);
    if (!State) {
        return;
    }
    if (!State) {
        return;
    }

    int AmmoNeeded = State->MaxAmmoInClip - State->AmmoInClip;
    if (AmmoNeeded <= 0) {
        return; // clip is full
    }

    int AmmoToReload = FMath::Min(AmmoNeeded, State->AmmoReserve);
    State->AmmoReserve = FMath::Max(0, State->AmmoReserve - AmmoToReload);
    State->AmmoInClip += AmmoToReload;
}

void UInventoryComponent::RemoveItem(EItemId ItemId) {
    if (!GetOwner() || !GetOwner()->HasAuthority())
        return;
    // Check and remove from weapon slots
    if (RifleState.ItemId == ItemId) {
        RifleState = FWeaponState(); // reset state
        OnRep_RifleState();
        return;
    }
    if (PistolState.ItemId == ItemId) {
        PistolState = FWeaponState(); // reset state
        OnRep_PistolState();
        return;
    }
    if (MeleeState.ItemId == ItemId) {
        MeleeState = FWeaponState(); // reset state
        OnRep_MeleeState();
        return;
    }
    // Check and remove from throwables
    int32 Removed = Throwables.Remove(ItemId);
    if (Removed > 0) {
        OnRep_Throwables();
        return;
    }
    // Check spike
    if (ItemId == EItemId::SPIKE && bHasSpike) {
        SetHasSpike(false);
        return;
    }
    // Check armor
    const UItemConfig* Data = GetItemConfig(ItemId);
    if (Data && Data->GetItemType() == EItemType::Armor) {
        ArmorState = FArmorState(); // reset armor state
        OnRep_ArmorState();
        return;
    }
}

bool UInventoryComponent::AddItemFromPickup(const FPickupData& PickupData)
{
    return AddItemInternal(PickupData);
}

bool UInventoryComponent::AddItemFromShop(const FPickupData& PickupData)
{
    return AddItemInternal(PickupData);
}

void UInventoryComponent::DropAllItems() {
    if (bHasSpike) {
        DropItem(EItemId::SPIKE);
        bHasSpike = false;
    }
    if (RifleState.ItemId != EItemId::NONE) {
        DropItem(RifleState.ItemId);
    }
    if (PistolState.ItemId != EItemId::NONE) {
        DropItem(PistolState.ItemId);
    }
}

APickupItem* UInventoryComponent::DropItem(EItemId Id) {
    if (Id == EItemId::NONE) {
        return nullptr;
    }

    FPickupData Data;
    Data.Location = GetOwner()->GetActorLocation();
    Data.Amount = 1;
    Data.ItemId = Id;
    Data.Id = UGameManager::Get(GetWorld())->GetNextItemOnMapId();

	const UItemConfig* ItemConfig = UItemsManager::Get(GetWorld())->GetItemById(Id);
	if (ItemConfig && ItemConfig->GetItemType() == EItemType::Firearm) {
		FWeaponState* WeaponState = GetWeaponStateByItemId(Id);
		if (WeaponState) {
			Data.AmmoInClip = WeaponState->AmmoInClip;
			Data.AmmoReserve = WeaponState->AmmoReserve;
		}
	}

    APickupItem* Pickup = UGameManager::Get(GetWorld())->CreatePickupActor(Data);

    ABaseCharacter* Character = Cast<ABaseCharacter>(this->GetOwner());
    if (Pickup)
    {
        Pickup->PlayerDropInfo(Character);
    }

    RemoveItem(Id);
	return Pickup;
}

void UInventoryComponent::ClearInventory() {
    RifleState.ItemId = EItemId::NONE;
    PistolState.ItemId = EItemId::NONE;
    Throwables.Empty();
}

void UInventoryComponent::OnBecomeHero() {
    MeleeState.ItemId = EItemId::MELEE_SWORD_HERO;
    OnRep_MeleeState();

	RifleState = FWeaponState();
	PistolState = FWeaponState();
	OnRep_RifleState();
	OnRep_PistolState();
	Throwables.Empty();
}

void UInventoryComponent::OnBecomeZombie() {
    MeleeState.ItemId = EItemId::MELEE_UNARMED_ZOMBIE;
    OnRep_MeleeState();

    RifleState = FWeaponState();
    PistolState = FWeaponState();
    OnRep_RifleState();
    OnRep_PistolState();
    Throwables.Empty();
}