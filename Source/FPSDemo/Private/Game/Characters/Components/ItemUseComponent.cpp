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

void UItemUseComponent::Init() {
    OwnerChar = Cast<ABaseCharacter>(GetOwner());
	check(OwnerChar);
   
    EquipComp = OwnerChar->GetEquipComponent();
    ActionStateComp = OwnerChar->GetActionStateComponent();
    WeaponFireComp = OwnerChar->GetWeaponFireComponent();
    WeaponMeleeComp = OwnerChar->GetWeaponMeleeComponent();
    ThrowableComp = OwnerChar->GetThrowableComponent();
	SpikeComp = OwnerChar->GetSpikeComponent();

	check(EquipComp);
	check(ActionStateComp);
	check(WeaponFireComp);
	check(WeaponMeleeComp);
	check(ThrowableComp);
	check(SpikeComp);
}

void UItemUseComponent::PrimaryPressed()
{
    if (!IsEnabled()) {
        return;
    }
    
    const UItemConfig* Item = EquipComp->GetActiveItemConfig();
    if (!Item) {
        return;
    }

    switch (Item->GetItemType())
    {
    case EItemType::Firearm:
        WeaponFireComp->RequestStartFire();
        break;

    case EItemType::Melee:
        WeaponMeleeComp->RequestMeleeAttack(FGameConstants::MELEE_ATTACK_INDEX_PRIMARY);
        break;

    case EItemType::Throwable:
        ThrowableComp->RequestStartThrow();
        break;

    default:
        if (Item->Id == EItemId::SPIKE) {       
            SpikeComp->RequestPlantSpike();
        }
        break;
    }
}

void UItemUseComponent::PrimaryReleased()
{
    if (!IsEnabled()) {
        return;
    }

    const UItemConfig* Item = EquipComp->GetActiveItemConfig();
    if (!Item) {
        return;
    }

    switch (Item->GetItemType())
    {
    case EItemType::Firearm:
        WeaponFireComp->RequestStopFire();
        break;
    default:
        if (Item->Id == EItemId::SPIKE)
        {
            SpikeComp->RequestStopPlantSpike();
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
        WeaponMeleeComp->RequestMeleeAttack(FGameConstants::MELEE_ATTACK_INDEX_SECONDARY);
        break;

    default:
        break;
    }
}

void UItemUseComponent::ReloadPressed() {
    if (!IsEnabled()) {
        return;
    }
    const UItemConfig* Item = EquipComp->GetActiveItemConfig();
    if (!Item || Item->GetItemType() != EItemType::Firearm)
    {
        return;
    }

    WeaponFireComp->RequestReload();
}