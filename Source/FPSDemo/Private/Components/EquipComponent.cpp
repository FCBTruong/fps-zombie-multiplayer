// ItemEquipComponent.cpp

#include "Components/EquipComponent.h"
#include "Game/GameManager.h"
#include "Components/ActionStateComponent.h"
#include "Weapons/WeaponDataManager.h" // or where UWeaponData is declared
#include "Components/InventoryComponent.h"

UEquipComponent::UEquipComponent()
{
    SetIsReplicatedByDefault(true);
}

void UEquipComponent::BeginPlay()
{
    Super::BeginPlay();

    if (AActor* Owner = GetOwner())
    {
        InventoryComp = Owner->FindComponentByClass<UInventoryComponent>();
        ActionStateComp = Owner->FindComponentByClass<UActionStateComponent>();
    }

    CachedGM = UGameManager::Get(GetWorld());
}

void UEquipComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UEquipComponent, ActiveItemId);
}

void UEquipComponent::OnRep_ActiveItemId()
{
    OnActiveItemChanged.Broadcast(ActiveItemId);
}

const UWeaponData* UEquipComponent::GetWeaponData(EItemId ItemId)
{
    if (ItemId == EItemId::NONE) return nullptr;

    if (!CachedGM)
    {
        CachedGM = UGameManager::Get(GetWorld());
    }

    // Your project stores weapon data by item id
    return CachedGM ? CachedGM->GetWeaponDataById(ItemId) : nullptr;
}

bool UEquipComponent::CanSelectNow() const
{
    // Single gate: only allow selecting while idle
    if (!ActionStateComp) return false;
    return ActionStateComp->IsIdle();
}

bool UEquipComponent::CanSelectItem(EItemId ItemId)
{
    if (!InventoryComp) return false;
    if (ItemId == EItemId::NONE) return false;

    const UWeaponData* Data = GetWeaponData(ItemId);
    if (!Data) return false;

    switch (Data->WeaponType)
    {
    case EWeaponTypes::Throwable:
        return InventoryComp->GetThrowables().Contains(ItemId);

    case EWeaponTypes::Spike:
        // Spike is active item only if player possesses it
        return InventoryComp->HasSpike() && ItemId == EItemId::SPIKE;

    case EWeaponTypes::Firearm:
    case EWeaponTypes::Melee:
        // Slot-bound weapons: only selectable if owned
        return (InventoryComp->GetRifleId() == ItemId) ||
            (InventoryComp->GetPistolId() == ItemId) ||
            (InventoryComp->GetMeleeId() == ItemId);

    default:
        return false;
    }
}

void UEquipComponent::Select_Internal(EItemId ItemId)
{
    if (!GetOwner() || !GetOwner()->HasAuthority())
        return;

    if (ActiveItemId == ItemId)
        return;

    ActiveItemId = ItemId;

    // Server doesn't receive OnRep
    OnActiveItemChanged.Broadcast(ActiveItemId);
}

void UEquipComponent::RequestSelectActiveItem(EItemId ItemId)
{
    if (!GetOwner()) return;

    if (!GetOwner()->HasAuthority())
    {
        ServerRequestSelectActiveItem(ItemId);
        return;
    }

    if (!CanSelectNow()) return;
    if (!CanSelectItem(ItemId)) return;

    Select_Internal(ItemId);
}

void UEquipComponent::ServerRequestSelectActiveItem_Implementation(EItemId ItemId)
{
    if (!CanSelectNow()) return;
    if (!CanSelectItem(ItemId)) return;

    Select_Internal(ItemId);
}

EItemId UEquipComponent::ChooseThrowableToSelect()
{
    if (!InventoryComp) return EItemId::NONE;

    const TArray<EItemId>& Throwables = InventoryComp->GetThrowables();
    if (Throwables.Num() == 0) return EItemId::NONE;

    const UWeaponData* CurrentData = GetWeaponData(ActiveItemId);
    if (!CurrentData || CurrentData->WeaponType != EWeaponTypes::Throwable)
    {
        return Throwables[0];
    }

    const int32 CurrentIndex = Throwables.IndexOfByKey(ActiveItemId);
    if (CurrentIndex == INDEX_NONE)
    {
        return Throwables[0];
    }

    const int32 NextIndex = (CurrentIndex + 1) % Throwables.Num();
    return Throwables[NextIndex];
}

void UEquipComponent::SelectSlot(int32 SlotIndex)
{
    if (!CanSelectNow()) return;
    if (!InventoryComp) return;

    EItemId Desired = EItemId::NONE;

    switch (SlotIndex)
    {
    case FGameConstants::SLOT_THROWABLE:
        Desired = ChooseThrowableToSelect();
        break;

    case FGameConstants::SLOT_RIFLE:
        Desired = InventoryComp->GetRifleId();
        break;

    case FGameConstants::SLOT_PISTOL:
        Desired = InventoryComp->GetPistolId();
        break;

    case FGameConstants::SLOT_MELEE:
        Desired = InventoryComp->GetMeleeId();
        break;

    case FGameConstants::SLOT_SPIKE:
        Desired = InventoryComp->HasSpike() ? EItemId::SPIKE : EItemId::NONE;
        break;

    default:
        break;
    }

    if (Desired == EItemId::NONE) return;

    RequestSelectActiveItem(Desired);
}

void UEquipComponent::AutoSelectBestWeapon()
{
    if (!CanSelectNow()) return;
    if (!InventoryComp) return;

    // Priority: Rifle > Pistol > Melee
    if (InventoryComp->GetRifleId() != EItemId::NONE)
    {
        RequestSelectActiveItem(InventoryComp->GetRifleId());
        return;
    }

    if (InventoryComp->GetPistolId() != EItemId::NONE)
    {
        RequestSelectActiveItem(InventoryComp->GetPistolId());
        return;
    }

    if (InventoryComp->GetMeleeId() != EItemId::NONE)
    {
        RequestSelectActiveItem(InventoryComp->GetMeleeId());
        return;
    }

    // If no weapons, keep NONE
    RequestSelectActiveItem(EItemId::NONE);
}
