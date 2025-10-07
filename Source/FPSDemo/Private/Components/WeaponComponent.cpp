// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/WeaponComponent.h"
#include "Characters/BaseCharacter.h"
#include "Weapons/WeaponDataManager.h"
#include "Weapons/WeaponFirearm.h"

// Sets default values for this component's properties
UWeaponComponent::UWeaponComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UWeaponComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UWeaponComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UWeaponComponent::EquipWeapon(AWeaponBase* NewWeapon)
{
	if (CurrentWeapon)
	{
		CurrentWeapon->Destroy();
	}
	CurrentWeapon = NewWeapon;
	if (CurrentWeapon)
	{
        EWeaponTypes WeaType = CurrentWeapon->GetWeaponType();

        FVector offset;
        FString SocketName = "ik_hand_gun";
		bool bIsFPS = true;
        if (WeaType == EWeaponTypes::Firearm) {
            if (bIsFPS) {
                SocketName = "ik_hand_gun";
                offset = FVector(0.f, 0.f, -6.f);
            }
            else {
                SocketName = "weapon_socket";
            }
        }
        else if (WeaType == EWeaponTypes::Melee) {
            SocketName = "melee_socket";
        }

        ABaseCharacter* Character = Cast<ABaseCharacter>(GetOwner());
        CurrentWeapon->AttachToComponent(Character->GetCurrentMesh(),
            FAttachmentTransformRules::SnapToTargetNotIncludingScale,
            FName(SocketName)
        );

        CurrentWeapon->SetActorRelativeLocation(offset);
	}
}


void UWeaponComponent::OnNewItemPickup(EItemId ItemId) {
    bool ShouldEquipNow = !CurrentWeapon or CurrentWeapon->GetWeaponType() != EWeaponTypes::Firearm;

    if (ShouldEquipNow) {
        // try get Item data from inventory
        UInventoryComponent* Inventory = GetOwner()->FindComponentByClass<UInventoryComponent>();
        UWeaponDataManager* WeaponDataMgr = GetOwner()->GetGameInstance()->GetSubsystem<UWeaponDataManager>();

        FActorSpawnParameters Params;
        Params.Owner = GetOwner();
        Params.Instigator = Cast<APawn>(GetOwner());
        Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        UWeaponData* WeaponData = Cast<UWeaponData>(WeaponDataMgr->GetWeaponById(ItemId));
        if (WeaponData)
        {
            AWeaponFirearm* WeaponFirearm = GetWorld()->SpawnActor<AWeaponFirearm>(
                AWeaponFirearm::StaticClass(),
                FVector::ZeroVector,
                FRotator::ZeroRotator,
                Params
            );
            WeaponFirearm->InitFromData(WeaponData); // optional setup
            EquipWeapon(WeaponFirearm);
        }
    }
}

EWeaponTypes UWeaponComponent::GetCurrentWeaponType() {
    if (CurrentWeapon) {
        return CurrentWeapon->GetWeaponType();
    }
    return EWeaponTypes::Unarmed;
}