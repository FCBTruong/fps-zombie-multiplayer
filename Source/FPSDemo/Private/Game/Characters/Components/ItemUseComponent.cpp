// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/Characters/Components/ItemUseComponent.h"
#include "Game/Characters/BaseCharacter.h"
#include "Shared/Data/Items/ItemConfig.h"
#include "Game/Characters/Components/EquipComponent.h"
#include "Game/Characters/Components/WeaponFireComponent.h"
#include "Game/Characters/Components/WeaponMeleeComponent.h"
#include "Game/Characters/Components/ThrowableComponent.h"
#include "Game/Characters/Components/SpikeComponent.h"

UItemUseComponent::UItemUseComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UItemUseComponent::BeginPlay() {
    Super::BeginPlay();

    OwnerChar = Cast<ABaseCharacter>(GetOwner());
    if (OwnerChar)
    {
        EquipComp = OwnerChar->GetEquipComponent();
        ActionStateComp = OwnerChar->GetActionStateComponent();
        WeaponFireComp = OwnerChar->GetWeaponFireComponent();
        WeaponMeleeComp = OwnerChar->GetWeaponMeleeComponent();
        ThrowableComp = OwnerChar->GetThrowableComponent();
		SpikeComp = OwnerChar->GetSpikeComponent();
    }
}

void UItemUseComponent::PrimaryPressed()
{
	UE_LOG(LogTemp, Log, TEXT("UItemUseComponent::PrimaryPressed called"));
    if (!IsEnabled()) {
		UE_LOG(LogTemp, Log, TEXT("UItemUseComponent: Not Enabled"));
        return;
    }
    if (!OwnerChar || !EquipComp) {
		UE_LOG(LogTemp, Log, TEXT("UItemUseComponent: No OwnerChar or EquipComp"));
        return;
    }
    const UItemConfig* Item = EquipComp->GetActiveItemConfig();
    if (!Item) {
		UE_LOG(LogTemp, Log, TEXT("UItemUseComponent: No Active Item"));
        return;
    }

	UE_LOG(LogTemp, Log, TEXT("UItemUseComponent:Active Item ID: %d"), static_cast<int32>(Item->Id)); 

    // print type
	UE_LOG(LogTemp, Log, TEXT("UItemUseComponent:Active Item Type: %d"), static_cast<int32>(Item->GetItemType()));
    switch (Item->GetItemType())
    {
    case EItemType::Firearm:
        UE_LOG(LogTemp, Log, TEXT("UItemUseComponent::debug00 called"));
        if (WeaponFireComp) {
            WeaponFireComp->RequestStartFire();
        }
        break;

    case EItemType::Melee:
        UE_LOG(LogTemp, Log, TEXT("UItemUseComponent::debug01 called"));
        if (WeaponMeleeComp) {
            UE_LOG(LogTemp, Log, TEXT("UItemUseComponent::debug02 called"));
            WeaponMeleeComp->RequestMeleeAttack(FGameConstants::MELEE_ATTACK_INDEX_PRIMARY);
        }
        break;

    case EItemType::Throwable:
        if (ThrowableComp) {
            ThrowableComp->RequestStartThrow();
        }
        break;

    default:
        if (Item->Id == EItemId::SPIKE) {
            if (SpikeComp) {
                SpikeComp->RequestPlantSpike();
            }
        }
        break;
    }
}

void UItemUseComponent::PrimaryReleased()
{
    if (!IsEnabled()) {
        return;
    }
    if (!OwnerChar || !EquipComp) return;

    const UItemConfig* Item = EquipComp->GetActiveItemConfig();
    if (!Item) return;

    switch (Item->GetItemType())
    {
    case EItemType::Firearm:
        if (WeaponFireComp)
        {
            WeaponFireComp->RequestStopFire();
        }
        break;
    default:
        // Spike is currently keyed by ItemId in your project
        if (Item->Id == EItemId::SPIKE)
        {
            if (SpikeComp) {
                SpikeComp->RequestStopPlantSpike();
            }
        }
        break;
    }
}

void UItemUseComponent::SecondaryReleased()
{
    if (!IsEnabled()) {
        return;
    }
}

void UItemUseComponent::SecondaryPressed()
{
    if (!IsEnabled()) {
        return;
	}
    if (!OwnerChar || !EquipComp) return;

    const UItemConfig* Item = EquipComp->GetActiveItemConfig();
    if (!Item) {
        return;
    }

    switch (Item->GetItemType())
    {
    case EItemType::Firearm:
        if (OwnerChar->IsAiming()) {
            OwnerChar->RequestStopAiming();
        }
        else {
            OwnerChar->RequestStartAiming();
        }
        break;

    case EItemType::Melee:
        if (WeaponMeleeComp)
        {
            WeaponMeleeComp->RequestMeleeAttack(FGameConstants::MELEE_ATTACK_INDEX_SECONDARY);
        }
        break;

    default:
        break;
    }
}

void UItemUseComponent::ReloadPressed() {
    if (!IsEnabled()) {
        return;
    }
    if (!OwnerChar || !EquipComp) return;

    const UItemConfig* Item = EquipComp->GetActiveItemConfig();
    if (!Item) {
        return;
    }

    if (Item->GetItemType() != EItemType::Firearm) {
        return;
    }

    if (WeaponFireComp) {
        WeaponFireComp->RequestReload();
    }
}