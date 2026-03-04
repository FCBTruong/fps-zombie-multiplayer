// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Game/Characters/Components/RoleGatedComponent.h"
#include "Net/UnrealNetwork.h"
#include "Shared/Types/ItemId.h"
#include "GameConstants.h"
#include "Shared/Types/EquippedAnimState.h"
#include "EquipComponent.generated.h"

class UInventoryComponent;
class UActionStateComponent;
class UItemConfig;
class UGameManager;
class ABaseCharacter;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnActiveItemChanged, EItemId);
DECLARE_MULTICAST_DELEGATE_TwoParams(
    FOnAmmoChanged,
    int32 /* AmmoInClip */,
    int32 /* AmmoInReserve */
);


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FPSDEMO_API UEquipComponent : public URoleGatedComponent
{
	GENERATED_BODY()

public:
    UEquipComponent();

    void Init();
    void RequestSelectActiveItem(EItemId ItemId);
    void SelectSlot(int32 SlotIndex);
    void AutoSelectBestWeapon();
    void RequestDropItem();
    void UnequipCurrentItem();
    bool GetCurrentAmmo(int32& OutClip, int32& OutReserve) const;
    const UItemConfig* GetActiveItemConfig() const;
    EEquippedAnimState GetEquippedAnimState() const;
    EItemId GetActiveItemId() const;
 
	// Delegates
    FOnActiveItemChanged OnActiveItemChanged;
	FOnAmmoChanged OnAmmoChanged;
protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnEnabledChanged(bool bNowEnabled) override;

private:
    // Replicated active item id (in hands)
    UPROPERTY(ReplicatedUsing = OnRep_ActiveItemId)
    EItemId ActiveItemId = EItemId::NONE;

    UPROPERTY()
    EEquippedAnimState CachedAnimState = EEquippedAnimState::Unarmed;

    // Dependencies (same owner actor)
    UInventoryComponent* InventoryComp = nullptr;
    UActionStateComponent* ActionStateComp = nullptr;
	ABaseCharacter* OwnerCharacter = nullptr;

private:
    // Helpers
    bool CanSelectNow() const;
    bool CanSelectItem(EItemId ItemId) const;
    bool CanDropItem() const;
    void RefreshCachedState();
    void Select_Internal(EItemId ItemId);
    void HandleDropItem();
    void RefreshOverlapPickupActors();
    void HandleAmmoDataChanged(EItemId ItemId, int32 Clip, int32 Reserve);
    void BroadcastActiveItemAndAmmo();
    const UItemConfig* GetItemConfig(EItemId ItemId) const;
    EItemId ChooseThrowableToSelect() const;

	UFUNCTION(Server, Reliable)
    void ServerDropItem();

    UFUNCTION()
    void OnRep_ActiveItemId();

    // RPC
    UFUNCTION(Server, Reliable)
    void ServerRequestSelectActiveItem(EItemId ItemId);
};