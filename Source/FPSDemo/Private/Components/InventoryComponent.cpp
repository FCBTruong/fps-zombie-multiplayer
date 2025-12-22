// WeaponInventoryComponent.cpp

#include "Components/InventoryComponent.h"
#include "Game/GameManager.h"
#include "Weapons/WeaponDataManager.h" 
#include "Weapons/WeaponState.h"       
#include "Structs/WeaponRuntimeData.h" 
#include "Pickup/PickupData.h"

UInventoryComponent::UInventoryComponent()
{
    SetIsReplicatedByDefault(true);

    // Default melee can be set by owner component that initializes loadout,
    // or here if you want fixed defaults.
}

void UInventoryComponent::BeginPlay()
{
    Super::BeginPlay();
    CachedGM = UGameManager::Get(GetWorld());
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

const UWeaponData* UInventoryComponent::GetWeaponData(EItemId ItemId)
{
    if (ItemId == EItemId::NONE) return nullptr;

    if (!CachedGM)
    {
        CachedGM = UGameManager::Get(GetWorld());
    }
    return CachedGM ? CachedGM->GetWeaponDataById(ItemId) : nullptr;
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
    const UWeaponData* Data = GetWeaponData(ItemId);
    return Data ? Data->CanDrop : false;
}

bool UInventoryComponent::AddItemFromPickup(const FPickupData& PickupData)
{
    if (!GetOwner() || !GetOwner()->HasAuthority())
        return false;

    const EItemId ItemId = PickupData.ItemId;
    const UWeaponData* Data = GetWeaponData(ItemId);
    if (!Data) return false;

    switch (Data->WeaponType)
    {
    case EWeaponTypes::Firearm:
    {
        if (Data->WeaponSubType == EWeaponSubTypes::Rifle)
        {
            if (RifleState.ItemId != EItemId::NONE) return false;
            RifleState.ItemId = ItemId;
            RifleState.AmmoInClip = PickupData.AmmoInClip;
            RifleState.AmmoReserve = PickupData.AmmoReserve;
            OnRep_RifleState();
            return true;
        }
        if (Data->WeaponSubType == EWeaponSubTypes::Pistol)
        {
            if (PistolState.ItemId != EItemId::NONE) return false;
            PistolState.ItemId = ItemId;
            PistolState.AmmoInClip = PickupData.AmmoInClip;
            PistolState.AmmoReserve = PickupData.AmmoReserve;
            OnRep_PistolState();
            return true;
        }
        return false;
    }

    case EWeaponTypes::Melee:
    {
        if (MeleeState.ItemId != EItemId::NONE) return false;
        MeleeState.ItemId = ItemId;
        // Melee ammo typically unused
        OnRep_MeleeState();
        return true;
    }

    case EWeaponTypes::Throwable:
    {
        Throwables.Add(ItemId);
        SortThrowables();
        OnRep_Throwables();
        return true;
    }

    case EWeaponTypes::Spike:
    {
        SetHasSpike(true);
        return true;
    }

    case EWeaponTypes::Equipment:
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
