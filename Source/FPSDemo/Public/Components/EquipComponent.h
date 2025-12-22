// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Net/UnrealNetwork.h"
#include "Items/ItemIds.h"
#include "GameConstants.h"
#include "EquipComponent.generated.h"

class UInventoryComponent;
class UActionStateComponent;
class UWeaponData;
class UGameManager;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnActiveItemChanged, EItemId);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FPSDEMO_API UEquipComponent : public UActorComponent
{
	GENERATED_BODY()

public:
    UEquipComponent();

    // Input-facing API
    void RequestSelectActiveItem(EItemId ItemId);
    void SelectSlot(int32 SlotIndex);
    void AutoSelectBestWeapon();

    EItemId GetActiveItemId() const { return ActiveItemId; }

    // Fired on server and clients when ActiveItemId changes
    FOnActiveItemChanged OnActiveItemChanged;

protected:
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
    // Replicated active item id (in hands)
    UPROPERTY(ReplicatedUsing = OnRep_ActiveItemId)
    EItemId ActiveItemId = EItemId::NONE;

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
    const UWeaponData* GetWeaponData(EItemId ItemId);

    // Authority-only
    void Select_Internal(EItemId ItemId);

    // Slot helpers
    EItemId ChooseThrowableToSelect();
};