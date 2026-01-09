// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/RoleGatedComponent.h"
#include "Net/UnrealNetwork.h"
#include "Items/ItemIds.h"
#include "GameConstants.h"
#include "Types/EquippedAnimState.h"
#include "EquipComponent.generated.h"

class UInventoryComponent;
class UActionStateComponent;
class UItemConfig;
class UGameManager;

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
	void Initialize(UInventoryComponent* InInventoryComp, UActionStateComponent* InActionStateComp);
    // Input-facing API
    void RequestSelectActiveItem(EItemId ItemId);
    void SelectSlot(int32 SlotIndex);
    void AutoSelectBestWeapon();
    bool GetCurrentAmmo(int32& OutClip, int32& OutReserve) const;
    const UItemConfig* GetActiveItemConfig() const;
    EEquippedAnimState GetEquippedAnimState() const { return CachedAnimState; }

    EItemId GetActiveItemId() const { return ActiveItemId; }

    // Fired on server and clients when ActiveItemId changes
    FOnActiveItemChanged OnActiveItemChanged;
	FOnAmmoChanged OnAmmoChanged;
    void RequestDropItem();
protected:
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnEnabledChanged(bool bNowEnabled) override;
private:
    // Replicated active item id (in hands)
    UPROPERTY(ReplicatedUsing = OnRep_ActiveItemId)
    EItemId ActiveItemId = EItemId::NONE;

    UPROPERTY()
    EEquippedAnimState CachedAnimState = EEquippedAnimState::Unarmed;

    UFUNCTION()
    void OnRep_ActiveItemId();

    // RPC
    UFUNCTION(Server, Reliable)
    void ServerRequestSelectActiveItem(EItemId ItemId);

private:
    // Dependencies (same owner actor)
    UPROPERTY() TObjectPtr<UInventoryComponent> InventoryComp = nullptr;
    UPROPERTY() TObjectPtr<UActionStateComponent> ActionStateComp = nullptr;

    UPROPERTY() 
    TObjectPtr<UGameManager> CachedGM = nullptr;

private:
    // Helpers
    bool CanSelectNow() const;
    bool CanSelectItem(EItemId ItemId);
    const UItemConfig* GetItemConfig(EItemId ItemId) const;

    // Authority-only
    void Select_Internal(EItemId ItemId);

    // Slot helpers
    EItemId ChooseThrowableToSelect();
    void RefreshCachedState();
	bool CanDropItem() const;

	UFUNCTION(Server, Reliable)
    void ServerDropItem();
	void HandleDropItem();
    void RefreshOverlapPickupActors();
    void HandleAmmoDataChanged(
        EItemId ItemId, int32 Clip, int32 Reserve);
    void BroadcastActiveItemAndAmmo();
};