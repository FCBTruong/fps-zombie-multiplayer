// WeaponInventoryComponent.cpp

#include "Game/Characters/Components/InventoryComponent.h"
#include "Game/GameManager.h"
#include "Game/Items/Weapons/WeaponState.h"       
#include "Game/Items/Pickup/PickupData.h"
#include "Shared/System/ItemsManager.h"
#include "Shared/Data/Items/ItemConfig.h"
#include "Shared/Data/Items/FirearmConfig.h"
#include "Net/UnrealNetwork.h"
#include "Game/Items/Pickup/PickupItem.h"
#include "Game/Characters/BaseCharacter.h"
#include "Shared/Data/Items/ArmorConfig.h"
#include "Game/Framework/ShooterGameMode.h"
#include "Game/Subsystems/ActorManager.h"

UInventoryComponent::UInventoryComponent()
{
    SetIsReplicatedByDefault(true);
    PrimaryComponentTick.bCanEverTick = false;
}

void UInventoryComponent::BeginPlay()
{
    Super::BeginPlay();

    Character = Cast<ABaseCharacter>(GetOwner());
	check(Character);
}

// Called on server when player finishes pre-match state, or when player respawns
void UInventoryComponent::InitBasicWeapon()
{
    if (!Character->HasAuthority())
    {
        return;
    }
	
	AShooterGameState* GameState = GetWorld()->GetGameState<AShooterGameState>();
    if (!GameState) {
		UE_LOG(LogTemp, Warning, TEXT("UInventoryComponent::InitBasicWeapon: GameState is null"));
        return;
    }
    if (GameState->GetMatchState() == EMyMatchState::PRE_MATCH)
    {
        return; // do not init in pre-match state
    }

    UItemsManager* ItemsManager = UItemsManager::Get(GetWorld());
    auto MatchMode = GameState->GetMatchMode();

    MeleeState.ItemId = EItemId::MELEE_KNIFE_BASIC;
    PistolState.ItemId = EItemId::PISTOL_PL_14;

	const UItemConfig* ItemPistol = ItemsManager->GetItemById(EItemId::PISTOL_PL_14);
	const UFirearmConfig* PistolData = Cast<UFirearmConfig>(ItemPistol);
    PistolState.AmmoInClip = PistolData ? PistolData->MaxAmmoInClip : 0;
    PistolState.AmmoReserve = PistolData ? PistolData->MaxAmmoInClip * 2 : 0;
	PistolState.MaxAmmoInClip = PistolData ? PistolData->MaxAmmoInClip : 0;

	const UItemConfig* ArmorItem = ItemsManager->GetItemById(EItemId::KEVLAR_VEST);
    const UArmorConfig* ArmorData = Cast<UArmorConfig>(ArmorItem);
    ArmorState.ArmorMaxPoints = ArmorData ? ArmorData->ArmorMaxPoints : 0;
    ArmorState.ArmorPoints = ArmorState.ArmorMaxPoints;
    ArmorState.ArmorEfficiency = ArmorData ? ArmorData->ArmorEfficiency : 0.f;
	ArmorState.ArmorRatio = ArmorData ? ArmorData->ArmorRatio : 0.f;
    Throwables.Add(EItemId::GRENADE_FRAG_BASIC);
 
    if (MatchMode == EMatchMode::DeathMatch || MatchMode == EMatchMode::Zombie)
    {
        TArray<EItemId> RifleOptions = {
            EItemId::RIFLE_AK_47,
            EItemId::RIFLE_M16A,
            EItemId::RIFLE_QBZ,
            EItemId::RIFLE_RUSSIAN_AS_VAL,
		};

		RifleState.ItemId = RifleOptions[FMath::RandRange(0, RifleOptions.Num() - 1)];
		const UItemConfig* ItemRifle = ItemsManager->GetItemById(RifleState.ItemId);
        const UFirearmConfig* RifleData = Cast<UFirearmConfig>(ItemRifle);
        RifleState.AmmoInClip = RifleData ? RifleData->MaxAmmoInClip : 0;
		RifleState.AmmoReserve = RifleData ? RifleData->MaxAmmoInClip * 3 : 0;
        RifleState.MaxAmmoInClip = RifleData ? RifleData->MaxAmmoInClip : 0;
    }

    OnRep_RifleState();
    OnRep_PistolState();
    OnRep_MeleeState();
    OnRep_ArmorState();
	OnRep_Throwables();
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

const UItemConfig* UInventoryComponent::GetItemConfig(EItemId ItemId) const
{
    if (ItemId == EItemId::NONE)
    {
        return nullptr;
    }

    UItemsManager* ItemsManager = UItemsManager::Get(GetWorld());
    return ItemsManager->GetItemById(ItemId);
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
    if (RifleState.ItemId == ItemId) {
        return &RifleState;
    }
    if (PistolState.ItemId == ItemId) {
        return &PistolState;
    }
    if (MeleeState.ItemId == ItemId) {
        return &MeleeState;
    }
    return nullptr;
}

const FWeaponState* UInventoryComponent::GetWeaponStateByItemId(EItemId ItemId) const
{
    if (RifleState.ItemId == ItemId) {
        return &RifleState;
    }
    if (PistolState.ItemId == ItemId) {
        return &PistolState;
    }
    if (MeleeState.ItemId == ItemId) {
        return &MeleeState;
    }
    return nullptr;
}

// Called by Server
bool UInventoryComponent::AddItemInternal(const FPickupData& PickupData)
{
    if (!Character->HasAuthority())
    {
        return false;
    }

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
			if (!FirearmData) {
				UE_LOG(LogTemp, Warning, TEXT("UInventoryComponent::AddItemInternal: ItemId %d is not a valid Firearm"), static_cast<int32>(ItemId));
				return false;
			}
            
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
            if (MeleeState.ItemId != EItemId::NONE) {
                return false;
            }
            MeleeState.ItemId = ItemId;
            // Melee ammo typically unused
            OnRep_MeleeState();
            return true;
        }
        case EItemType::Throwable:
        {
			// check if already have this throwable
            if (Throwables.Contains(ItemId))
            {
                return false;
            }

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

// Called by Server when player throws or drops throwable
bool UInventoryComponent::RemoveThrowable(EItemId ItemId)
{
    if (!Character->HasAuthority())
    {
        return false;
    }

    const int32 Removed = Throwables.Remove(ItemId);
    if (Removed > 0)
    {
        OnRep_Throwables();
        return true;
    }
    return false;
}

// Called by Server when player picks up or drops spike
void UInventoryComponent::SetHasSpike(bool bNewHasSpike)
{
    if (!Character->HasAuthority())
    {
        return;
    }
    if (bHasSpike == bNewHasSpike)
    {
        return;
    }

    bHasSpike = bNewHasSpike;
    OnRep_HasSpike();
}

// Called by Server when player equips armor
void UInventoryComponent::ApplyArmorItem(EItemId ArmorItemId)
{
    if (!Character->HasAuthority())
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

// Called by Server when player shoots weapon
bool UInventoryComponent::ConsumeAmmo(EItemId WeaponId, int32 Amount)
{
    if (!Character->HasAuthority())
    {
        return false;
    }

    if (Amount <= 0) {
        return true;
    }

    FWeaponState* WS = GetWeaponStateByItemId(WeaponId);
    if (!WS) {
		UE_LOG(LogTemp, Warning, TEXT("UInventoryComponent::ConsumeAmmo: WeaponId %d not found in inventory"), static_cast<int32>(WeaponId));
        return false;
    }
    WS->AmmoInClip = FMath::Max(0, WS->AmmoInClip - Amount);
    
    // On Rep manually
    if (WS == &RifleState)
    {
        OnRep_RifleState();
    }
    else if (WS == &PistolState)
    {
        OnRep_PistolState();
    }

    return true;
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

// Called by Server
void UInventoryComponent::ReloadWeapon(EItemId Id) {
    if (!Character->HasAuthority())
    {
        return;
    }

    FWeaponState* State = GetWeaponStateByItemId(Id);
    if (!State) {
        return;
    }
  
    int32 AmmoNeeded = State->MaxAmmoInClip - State->AmmoInClip;
    if (AmmoNeeded <= 0) {
        return; // clip is full
    }

    int32 AmmoToReload = FMath::Min(AmmoNeeded, State->AmmoReserve);
    State->AmmoReserve = FMath::Max(0, State->AmmoReserve - AmmoToReload);
    State->AmmoInClip += AmmoToReload;

    if (State == &RifleState)
    {
        OnRep_RifleState();
    }
    else if (State == &PistolState)
    {
        OnRep_PistolState();
    }
}

// Called by Server
bool UInventoryComponent::RemoveItem(EItemId ItemId) {
    if (!Character->HasAuthority())
    {
        return false;
    }
    // Check and remove from weapon slots
    if (RifleState.ItemId == ItemId) {
        RifleState = FWeaponState(); // reset state
        OnRep_RifleState();
        return true;
    }
    if (PistolState.ItemId == ItemId) {
        PistolState = FWeaponState(); // reset state
        OnRep_PistolState();
        return true;
    }
    if (MeleeState.ItemId == ItemId) {
        MeleeState = FWeaponState(); // reset state
        OnRep_MeleeState();
        return true;
    }
    // Check and remove from throwables
    if (Throwables.RemoveSingle(ItemId) > 0)
    {
        OnRep_Throwables();
        return true;
    }
    // Check spike
    if (ItemId == EItemId::SPIKE && bHasSpike) {
        SetHasSpike(false);
        return true;
    }
    // Check armor
    const UItemConfig* Data = GetItemConfig(ItemId);
    if (Data && Data->GetItemType() == EItemType::Armor) {
        ArmorState = FArmorState(); // reset armor state
        OnRep_ArmorState();
        return true;
    }
    return false;
}

// Called by Server
bool UInventoryComponent::AddItemFromPickup(const FPickupData& PickupData)
{
    return AddItemInternal(PickupData);
}

// Called by Server
bool UInventoryComponent::AddItemFromShop(const FPickupData& PickupData)
{
    return AddItemInternal(PickupData);
}

// Called by Server
void UInventoryComponent::DropAllItems() {
    if (!Character->HasAuthority())
    {
        return;
    }

    if (bHasSpike) {
        DropItem(EItemId::SPIKE);
    }
    if (RifleState.ItemId != EItemId::NONE) {
        DropItem(RifleState.ItemId);
    }
    const TArray<EItemId> ThrowablesCopy = Throwables;
    for (EItemId ThrowableId : ThrowablesCopy)
    {
        DropItem(ThrowableId);
    }
}

// Called by Server
APickupItem* UInventoryComponent::DropItem(EItemId Id) {
    if (!Character->HasAuthority())
    {
        return nullptr;
    }

    if (Id == EItemId::NONE) {
        return nullptr;
    }

	AActorManager* ActorMgr = AActorManager::Get(GetWorld());
    if (!ActorMgr) {
        return nullptr;
	}

    FPickupData Data;
    Data.Location = Character->GetActorLocation();
    Data.Amount = 1;
    Data.ItemId = Id;
    Data.Id = ActorMgr->GetNextItemOnMapId();

	const UItemConfig* ItemConfig = GetItemConfig(Id);
	if (ItemConfig && ItemConfig->GetItemType() == EItemType::Firearm) {
		FWeaponState* WeaponState = GetWeaponStateByItemId(Id);
		if (WeaponState) {
			Data.AmmoInClip = WeaponState->AmmoInClip;
			Data.AmmoReserve = WeaponState->AmmoReserve;
		}
	}

	bool bDidRemove = RemoveItem(Id);
    if (!bDidRemove) {
		UE_LOG(LogTemp, Warning, TEXT("UInventoryComponent::DropItem: Failed to remove item %d from inventory"), static_cast<int32>(Id));
        return nullptr;
	}
    APickupItem* Pickup = ActorMgr->CreatePickupActor(Data);
    if (!Pickup)
    {
        return nullptr;
    }

    Pickup->RecordDropInfo(Character);
	Pickup->SetIsActive(true);
	return Pickup;
}

// Called by Server
void UInventoryComponent::OnBecomeHero() {
    MeleeState.ItemId = EItemId::MELEE_SWORD_HERO;
	RifleState = FWeaponState();
	PistolState = FWeaponState();
	Throwables.Empty();

    OnRep_RifleState();
    OnRep_PistolState();
	OnRep_Throwables();
    OnRep_MeleeState();
}

// Called by Server
void UInventoryComponent::OnBecomeZombie() {
    MeleeState.ItemId = EItemId::MELEE_UNARMED_ZOMBIE;
    RifleState = FWeaponState();
    PistolState = FWeaponState();
    Throwables.Empty();

    OnRep_RifleState();
    OnRep_PistolState();
    OnRep_MeleeState();
	OnRep_Throwables();
}

// Called by Server
void UInventoryComponent::AddAmmoToMainGun(int32 Ammo) {
    // Still need check since it updates replicated state
    if (!Character->HasAuthority())
    {
        return;
    }

    if (RifleState.ItemId != EItemId::NONE) {
		RifleState.AmmoReserve += Ammo;
		OnRep_RifleState();
	}
    else if (PistolState.ItemId != EItemId::NONE) {
        PistolState.AmmoReserve += Ammo;
        OnRep_PistolState();
    }
}

// Called by Server
void UInventoryComponent::UpdateArmorPoints(int NewArmorPoints) {
    if (!Character->HasAuthority())
    {
        return;
    }
    ArmorState.ArmorPoints = FMath::Clamp(NewArmorPoints, 0, ArmorState.ArmorMaxPoints);
    OnRep_ArmorState();
}