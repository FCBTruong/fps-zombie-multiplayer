// WeaponInventoryComponent.cpp

#include "Components/InventoryComponent.h"
#include "Game/GameManager.h"
#include "Weapons/WeaponState.h"       
#include "Structs/WeaponRuntimeData.h" 
#include "Pickup/PickupData.h"
#include "Game/ItemsManager.h"
#include "Items/ItemConfig.h"
#include "Items/FirearmConfig.h"
#include "Net/UnrealNetwork.h"

UInventoryComponent::UInventoryComponent()
{
    SetIsReplicatedByDefault(true);
}

void UInventoryComponent::BeginPlay()
{
    Super::BeginPlay();
    CachedGM = UGameManager::Get(GetWorld());
}

void UInventoryComponent::Test()
{
    // Test function to be removed later
    MeleeState.ItemId = EItemId::MELEE_KNIFE_BASIC;
    PistolState.ItemId = EItemId::PISTOL_PL_14;
	RifleState.ItemId = EItemId::RIFLE_AK_47;

    UWeaponData* PistolData = UGameManager::Get(GetWorld())->GetWeaponDataById(EItemId::PISTOL_PL_14);
    PistolState.AmmoInClip = PistolData ? PistolData->MaxAmmoInClip : 0;
    PistolState.AmmoReserve = PistolData ? PistolData->MaxAmmoInClip * 2 : 0;
	PistolState.MaxAmmoInClip = PistolData ? PistolData->MaxAmmoInClip : 0;

	UWeaponData* RifleData = UGameManager::Get(GetWorld())->GetWeaponDataById(RifleState.ItemId);
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

bool UInventoryComponent::AddItemFromPickup(const FPickupData& PickupData)
{
    if (!GetOwner() || !GetOwner()->HasAuthority())
        return false;

    const EItemId ItemId = PickupData.ItemId;
    const UItemConfig* Data = GetItemConfig(ItemId);
    if (!Data) return false;

    EItemType ItemType = Data->GetItemType();
    switch (ItemType)
    {
        case EItemType::Firearm:
        {
			const UFirearmConfig* FirearmData = Cast<UFirearmConfig>(Data);
            
            if (FirearmData->FirearmType == EFirearmType::Rifle)
            {
                if (RifleState.ItemId != EItemId::NONE) return false;
                RifleState.ItemId = ItemId;
                RifleState.AmmoInClip = PickupData.AmmoInClip;
                RifleState.AmmoReserve = PickupData.AmmoReserve;
                OnRep_RifleState();
                return true;
            }
            else 
            {
                if (PistolState.ItemId != EItemId::NONE) return false;
                PistolState.ItemId = ItemId;
                PistolState.AmmoInClip = PickupData.AmmoInClip;
                PistolState.AmmoReserve = PickupData.AmmoReserve;
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
        case EItemType::Spike:
        {
            SetHasSpike(true);
            return true;
        }
        case EItemType::Armor:
        {
            ApplyArmorItem(ItemId);
            return true;
        }

        default:
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
        return;

    // Keep your existing hard-coded test logic
    if (ArmorItemId == EItemId::KEVLAR_VEST)
    {
        ArmorState.ArmorPoints = 100;
        ArmorState.ArmorEfficiency = 0.4f;
        ArmorState.ArmorRatio = 0.3f;
    }
    else
    {
        ArmorState.ArmorPoints = 100;
        ArmorState.ArmorEfficiency = 0.3f;
        ArmorState.ArmorRatio = 0.5f;
    }

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
    // No OnRep here; caller decides when to refresh UI (or you can broadcast)
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
}

void UInventoryComponent::OnRep_PistolState()
{
    OnPistolChanged.Broadcast(PistolState.ItemId);
    OnInventoryChanged.Broadcast();
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
    OnArmorChanged.Broadcast(ArmorState.ArmorPoints);
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