// ItemEquipComponent.cpp

#include "Components/EquipComponent.h"
#include "Game/GameManager.h"
#include "Components/ActionStateComponent.h"
#include "Components/InventoryComponent.h"
#include "Game/ItemsManager.h"
#include "Items/ItemConfig.h"

UEquipComponent::UEquipComponent()
{
    SetIsReplicatedByDefault(true);
}

void UEquipComponent::Initialize(UInventoryComponent* InInventoryComp, UActionStateComponent* InActionStateComp) {
    InventoryComp = InInventoryComp;
    ActionStateComp = InActionStateComp;
}

void UEquipComponent::BeginPlay()
{
    Super::BeginPlay();

    CachedGM = UGameManager::Get(GetWorld());
}

void UEquipComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(UEquipComponent, ActiveItemId);
}

void UEquipComponent::OnRep_ActiveItemId()
{
    RefreshCachedState();
	UE_LOG(LogTemp, Log, TEXT("UEquipComponent::OnRep_ActiveItemId: New ActiveItemId = %d"), static_cast<int32>(ActiveItemId));
    OnActiveItemChanged.Broadcast(ActiveItemId);
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

    const UItemConfig* Data = GetItemConfig(ItemId);
    if (!Data) return false;

    if (Data->Id == EItemId::SPIKE) {
        return InventoryComp->HasSpike();
	}

    if (Data->GetItemType() == EItemType::Firearm)
    {
        return (InventoryComp->GetRifleId() == ItemId) ||
            (InventoryComp->GetPistolId() == ItemId);
    }
    else if (Data->GetItemType() == EItemType::Melee) {
		return (InventoryComp->GetMeleeId() == ItemId);
    }
	else if (Data->GetItemType() == EItemType::Throwable) {
		return InventoryComp->GetThrowables().Contains(ItemId);
	}
  
    return false;
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

    if (!CanSelectNow()) {
        // log debug
		UE_LOG(LogTemp, Warning, TEXT("UEquipComponent::RequestSelectActiveItem: Cannot select item now."));
        return;
    }
    if (!CanSelectItem(ItemId)) {
		// log debug
		UE_LOG(LogTemp, Warning, TEXT("UEquipComponent::RequestSelectActiveItem: Cannot select item %d."), static_cast<int32>(ItemId));
        return;
    }

    if (!GetOwner()->HasAuthority())
    {
        ServerRequestSelectActiveItem(ItemId);
        return;
    }

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
	UE_LOG(LogTemp, Log, TEXT("UEquipComponent::ChooseThrowableToSelect called"));
    if (!InventoryComp) return EItemId::NONE;

    const TArray<EItemId>& Throwables = InventoryComp->GetThrowables();

    if (Throwables.Num() == 0) return EItemId::NONE;

    const UItemConfig* CurrentData = GetActiveItemConfig();
    if (!CurrentData || CurrentData->GetItemType() != EItemType::Throwable)
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


const UItemConfig* UEquipComponent::GetItemConfig(EItemId ItemId) const
{
    if (ItemId == EItemId::NONE) return nullptr;
    return UItemsManager::Get(GetWorld())->GetItemById(ItemId);
}


const UItemConfig* UEquipComponent::GetActiveItemConfig() const
{
    return GetItemConfig(ActiveItemId);
}

void UEquipComponent::RefreshCachedState()
{
    const UItemConfig* Cfg = GetActiveItemConfig();
    CachedAnimState = Cfg ? Cfg->AnimationState : EEquippedAnimState::Unarmed;
}