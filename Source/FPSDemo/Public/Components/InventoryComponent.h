// WeaponInventoryComponent.h

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Items/ItemIds.h"
#include "Weapons/WeaponState.h"
#include "InventoryComponent.generated.h"

class UItemConfig;
class UGameManager;
struct FPickupData;
class APickupItem;

DECLARE_MULTICAST_DELEGATE(FOnInventoryChanged);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnAmmoDataChanged, EItemId, int /*InClip*/, int /*Reserve*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnThrowablesChanged, const TArray<EItemId>&);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnArmorChanged, int, int);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnSpikeChanged, bool /*bHasSpike*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnSlotWeaponChanged, EItemId /*ItemId*/);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class FPSDEMO_API UInventoryComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UInventoryComponent();

    // ===== Queries =====
    const TArray<EItemId>& GetThrowables() const { return Throwables; }
    bool HasSpike() const { return bHasSpike; }

    EItemId GetRifleId() const { return RifleState.ItemId; }
    EItemId GetPistolId() const { return PistolState.ItemId; }
    EItemId GetMeleeId() const { return MeleeState.ItemId; }

    const FWeaponState& GetRifleState() const { return RifleState; }
    const FWeaponState& GetPistolState() const { return PistolState; }
    const FWeaponState& GetMeleeState() const { return MeleeState; }
    const FArmorState& GetArmorState() const { return ArmorState; }
	FArmorState& GetArmorStateMutable() { return ArmorState; }

    FWeaponState* GetWeaponStateByItemId(EItemId ItemId);
    const FWeaponState* GetWeaponStateByItemId(EItemId ItemId) const;
	bool AddItemFromShop(const FPickupData& PickupData); // add item from shop (server only)
    bool AddItemFromPickup(const FPickupData& PickupData);

    // ===== Mutations (Authority only) =====
    // Add item to inventory from pickup data (server only)
    

    void InitBasicWeapon();

    // Remove throwable after throw/drop (server only)
    bool RemoveThrowable(EItemId ItemId);

    // Spike possession (server only)
    void SetHasSpike(bool bNewHasSpike);

    // Armor (server only)
    void ApplyArmorItem(EItemId ArmorItemId);

    // Ammo operations (server only)
    bool ConsumeAmmo(EItemId WeaponId, int32 Amount);

    // Drop rules (pure query)
    bool CanDrop(EItemId ItemId);
    void ReloadWeapon(EItemId Id); // reload current weapon
	void RemoveItem(EItemId ItemId); // remove item from inventory (server only)
	void DropAllItems(); // drop all items (server only)
    APickupItem* DropItem(EItemId ItemId);
	void OnBecomeHero(); // this will change melee to hero sword
    void OnBecomeZombie();
	int GetArmorPoints() const { return ArmorState.ArmorPoints; }
	int GetArmorMaxPoints() const { return ArmorState.ArmorMaxPoints; }

    void ClearInventory();
public:
    // ===== Events (fire on server + clients via OnRep) =====
    FOnInventoryChanged OnInventoryChanged;
    FOnThrowablesChanged OnThrowablesChanged;
    FOnArmorChanged OnArmorChanged;
    FOnSpikeChanged OnSpikeChanged;
    FOnSlotWeaponChanged OnRifleChanged;
    FOnSlotWeaponChanged OnPistolChanged;
    FOnSlotWeaponChanged OnMeleeChanged;
    FOnAmmoDataChanged OnAmmoDataChanged;

protected:
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
    // ===== Replicated State =====
    UPROPERTY(ReplicatedUsing = OnRep_RifleState)
    FWeaponState RifleState;

    UPROPERTY(ReplicatedUsing = OnRep_PistolState)
    FWeaponState PistolState;

    UPROPERTY(ReplicatedUsing = OnRep_MeleeState)
    FWeaponState MeleeState;

    UPROPERTY(ReplicatedUsing = OnRep_Throwables)
    TArray<EItemId> Throwables;

    UPROPERTY(ReplicatedUsing = OnRep_ArmorState)
    FArmorState ArmorState;

    UPROPERTY(ReplicatedUsing = OnRep_HasSpike)
    bool bHasSpike = false;

private:
    // Optional cache
    UPROPERTY() TObjectPtr<UGameManager> CachedGM = nullptr;

private:
    // Helpers
    const UItemConfig* GetItemConfig(EItemId ItemId);
    void SortThrowables();
    bool AddItemInternal(const FPickupData& PickupData);

    // OnRep
    UFUNCTION() void OnRep_RifleState();
    UFUNCTION() void OnRep_PistolState();
    UFUNCTION() void OnRep_MeleeState();
    UFUNCTION() void OnRep_Throwables();
    UFUNCTION() void OnRep_ArmorState();
    UFUNCTION() void OnRep_HasSpike();
};
