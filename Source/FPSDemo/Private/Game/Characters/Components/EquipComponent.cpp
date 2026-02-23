// ItemEquipComponent.cpp

#include "Game/Characters/Components/EquipComponent.h"
#include "Game/GameManager.h"
#include "Game/Characters/Components/ActionStateComponent.h"
#include "Game/Characters/Components/InventoryComponent.h"
#include "Shared/System/ItemsManager.h"
#include "Shared/Data/Items/ItemConfig.h"
#include "Game/Characters/BaseCharacter.h"
#include <Kismet/GameplayStatics.h>
#include "Game/Modes/Spike/SpikeMode.h"
#include "Components/CapsuleComponent.h"
#include "Game/Characters/Components/PickupComponent.h"
#include "Game/Items/Pickup/PickupItem.h"

UEquipComponent::UEquipComponent()
{
    SetIsReplicatedByDefault(true);
}

void UEquipComponent::BeginPlay()
{
    Super::BeginPlay();
	UE_LOG(LogTemp, Log, TEXT("UEquipComponent::BeginPlay called"));

    CachedGM = UGameManager::Get(GetWorld());
	ABaseCharacter* OwnerChar = Cast<ABaseCharacter>(GetOwner());
    if (OwnerChar) {
		InventoryComp = OwnerChar->GetInventoryComponent();
		ActionStateComp = OwnerChar->GetActionStateComponent();
	}
    if (InventoryComp) {
        InventoryComp->OnAmmoDataChanged.AddUObject(
            this, &UEquipComponent::HandleAmmoDataChanged);
    }
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
    BroadcastActiveItemAndAmmo();
}

bool UEquipComponent::CanSelectNow() const
{
    if (!IsEnabled()) {
        return false;
    }
    // Single gate: only allow selecting while idle
    if (!ActionStateComp) return false;
    return ActionStateComp->IsIdle();
}

bool UEquipComponent::CanSelectItem(EItemId ItemId)
{
    if (!IsEnabled()) {
        return false;
    }
    if (!InventoryComp) {
		UE_LOG(LogTemp, Warning, TEXT("UEquipComponent::CanSelectItem: InventoryComp not found"));
        return false;
    }
    if (ItemId == EItemId::NONE) {
        return false;
    }

    const UItemConfig* Data = GetItemConfig(ItemId);
    if (!Data) {
		UE_LOG(LogTemp, Warning, TEXT("UEquipComponent::CanSelectItem: ItemConfig not found for ItemId %d"), static_cast<int32>(ItemId));
        return false;
    }

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
    if (!IsEnabled()) {
        return;
    }
    if (!GetOwner() || !GetOwner()->HasAuthority())
        return;

    if (ActiveItemId == ItemId)
        return;

    ActiveItemId = ItemId;

    // Server doesn't receive OnRep
    BroadcastActiveItemAndAmmo();
}

void UEquipComponent::RequestSelectActiveItem(EItemId ItemId)
{
    if (!IsEnabled()) {
        return;
    }
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

EItemId UEquipComponent::ChooseThrowableToSelect() const
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
    if (!CanSelectNow()) {
        UE_LOG(LogTemp, Warning, TEXT("SelectSlot: Error can not select now"))
        return;
    }
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
    if (!IsEnabled()) {
        return;
    }
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

bool UEquipComponent::CanDropItem() const {
    if (!IsEnabled()) {
        return false;
    }
    const UItemConfig* ItemConfig = GetActiveItemConfig();
    if (!ItemConfig) {
        return false;
    }
    if (!ItemConfig->bIsDroppable) {
        UE_LOG(LogTemp, Warning, TEXT("CanDropItem: Current item is not droppable"));
        return false;
    }
    if (ActionStateComp && !ActionStateComp->IsIdle()) {
        UE_LOG(LogTemp, Warning, TEXT("CanDropItem: Cannot drop item while not idle"));
        return false;
	}
    return true;
}

void UEquipComponent::RequestDropItem() {
    if (!IsEnabled()) {
        return;
    }
    UE_LOG(LogTemp, Warning, TEXT("DropItem called"));
    if (CanDropItem()) {
        if (!GetOwner()->HasAuthority()) {
            ServerDropItem();
        }
        else {
            HandleDropItem();
        }
    }
}

void UEquipComponent::ServerDropItem_Implementation() {
    if (!IsEnabled()) {
        return;
    }
    HandleDropItem();
}


// Server function
void UEquipComponent::HandleDropItem() {
    if (!CanDropItem()) {
        UE_LOG(LogTemp, Warning, TEXT("HandleDropItem: Current item can not be dropped"));
        return;
    }
	const UItemConfig* ItemConf = GetActiveItemConfig();

    // spawn new pickup item on map
	ABaseCharacter* Character = Cast<ABaseCharacter>(GetOwner());
    if (!Character) {
        UE_LOG(LogTemp, Warning, TEXT("HandleDropWeapon: Owner is not ABaseCharacter"));
        return;
	}
    FVector DropPoint = GetOwner()->GetActorLocation() + FVector(0.f, 0.f, 60.f) + Character->GetActorForwardVector() * 30;
    

    FVector LookDir = Character->GetControlRotation().Vector();
    FVector LaunchVelocity = LookDir * 600.f;

    // Spawn Pickup item
    APickupItem* Pickup = InventoryComp->DropItem(ActiveItemId);
	Pickup->SetActorLocation(DropPoint);

    if (Pickup && Pickup->GetItemMesh())
    {
        Pickup->GetItemMesh()->AddImpulse(LaunchVelocity, NAME_None, true);
    }

    ActiveItemId = EItemId::NONE;
	OnRep_ActiveItemId();
 
    // refresh overlapping actors
    RefreshOverlapPickupActors();

    AutoSelectBestWeapon();
}

void UEquipComponent::RefreshOverlapPickupActors() {
	ABaseCharacter* Character = Cast<ABaseCharacter>(GetOwner());
	if (!Character) return;

    if (Character->HasAuthority())
    {
        return; // server-only
    }

    UCapsuleComponent* Cap = Character->GetCapsuleComponent();
    TArray<AActor*> OverlappingActors;
    Cap->GetOverlappingActors(OverlappingActors);
    for (AActor* A : OverlappingActors)
    {
        APickupItem* Item = Cast<APickupItem>(A);
        if (Item && !Item->IsJustDropped(Character))
        {
            UE_LOG(LogTemp, Warning, TEXT("Overlapping PickupItem: %s"), *Item->GetName());
            // call pickup component to manually trigger overlap
            UPickupComponent* PickupComp = Character->GetPickupComponent();
            if (PickupComp) {
                PickupComp->PickupItem(Item);
            }
        }
    }
}

void UEquipComponent::HandleAmmoDataChanged(
    EItemId ItemId, int32 Clip, int32 Reserve)
{
    if (ItemId != ActiveItemId)
        return;

    OnAmmoChanged.Broadcast(Clip, Reserve);
}

bool UEquipComponent::GetCurrentAmmo(int32& OutClip, int32& OutReserve) const
{
    OutClip = 0;
    OutReserve = 0;

    if (!InventoryComp)
    {
        return false;
    }

    if (ActiveItemId == EItemId::NONE)
    {
        return false;
    }

    const FWeaponState* State = InventoryComp->GetWeaponStateByItemId(ActiveItemId);
    if (!State)
    {
        return false;
    }

    OutClip = State->AmmoInClip;
    OutReserve = State->AmmoReserve;
    return true;
}


void UEquipComponent::BroadcastActiveItemAndAmmo()
{
    RefreshCachedState();
    OnActiveItemChanged.Broadcast(ActiveItemId);

    int32 Clip = 0;
    int32 Reserve = 0;
    if (GetCurrentAmmo(Clip, Reserve))
    {
        OnAmmoChanged.Broadcast(Clip, Reserve);
    }
}

void UEquipComponent::OnEnabledChanged(bool bNowEnabled)
{
    if (!bNowEnabled)
    {
        if (GetOwner() && GetOwner()->HasAuthority())
        {
            ActiveItemId = EItemId::NONE;
            BroadcastActiveItemAndAmmo();
        }
    }
}

void UEquipComponent::UnequipCurrentItem()
{
    if (!IsEnabled()) {
        return;
    }
    if (!GetOwner()->HasAuthority())
        return;
    if (ActiveItemId == EItemId::NONE)
        return;
    ActiveItemId = EItemId::NONE;
	OnRep_ActiveItemId();
}

EEquippedAnimState UEquipComponent::GetEquippedAnimState() const {
    return CachedAnimState; 
}

EItemId UEquipComponent::GetActiveItemId() const { 
    return ActiveItemId;
}