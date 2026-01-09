// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/ItemUseComponent.h"
#include "Characters/BaseCharacter.h"
#include "Items/ItemConfig.h"
#include "Components/EquipComponent.h"
#include "Components/WeaponFireComponent.h"
#include "Components/WeaponMeleeComponent.h"
#include "Components/ThrowableComponent.h"
#include "Components/SpikeComponent.h"

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
    }
}

void UItemUseComponent::PrimaryPressed()
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
        if (WeaponFireComp) {
            WeaponFireComp->RequestStartFire();
        }
        break;

    case EItemType::Melee:
        if (WeaponMeleeComp) {
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