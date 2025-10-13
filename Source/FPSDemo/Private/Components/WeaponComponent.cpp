// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/WeaponComponent.h"
#include "Characters/BaseCharacter.h"
#include "Weapons/WeaponDataManager.h"
#include "Weapons/WeaponFirearm.h"
#include "Characters/PlayerCharacter.h"

// Sets default values for this component's properties
UWeaponComponent::UWeaponComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicated(true);
}


// Called when the game starts
void UWeaponComponent::BeginPlay()
{
	Super::BeginPlay();

    GMR = GetWorld()->GetGameInstance()->GetSubsystem<UGameManager>();
}


// Called every frame
void UWeaponComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UWeaponComponent::EquipWeapon(int32 InventoryId)
{
	ServerEquipWeapon(InventoryId);

    if (GetWorld()->GetNetMode() == NM_ListenServer) {
        // Host / Listen server
        
		// For host, we need to manually call the OnRep function since replication won't trigger it locally
        OnRep_CurrentInventoryId();
    }
}


void UWeaponComponent::ServerEquipWeapon_Implementation(int32 InventoryId)
{
	UE_LOG(LogTemp, Warning, TEXT("ServerEquipWeapon_Implementation called withaaaa InventoryId: %d"), InventoryId);
    CurrentInventoryId = InventoryId;
}

// Client function, handles weapon spawning and attaching
void UWeaponComponent::OnRep_CurrentInventoryId()
{
    UE_LOG(LogTemp, Warning, TEXT("OnRep_CurrentInventoryId called with"));
	// Gen new Weapon based on InventoryId
    if (CurrentInventoryId <= 0) {
		UE_LOG(LogTemp, Warning, TEXT("OnRep_CurrentInventoryId: Invalid InventoryId"));
        return;
	}

	UInventoryComponent* Inventory = GetOwner()->FindComponentByClass<UInventoryComponent>();
	if (!Inventory) {
		UE_LOG(LogTemp, Warning, TEXT("OnRep_CurrentInventoryId: Inventory component not found"));
		return;
	}
	// debug ,print size inventory
	int32 x = Inventory->GetItemCount();
    UE_LOG(LogTemp, Warning, TEXT("OnRep_CurrentInventoryId: Inventory has %d items"), x);
 
	FInventoryItem* Item = Inventory->GetItemByInventoryId(CurrentInventoryId);
	if (!Item) {
		UE_LOG(LogTemp, Warning, TEXT("OnRep_CurrentInventoryId: Item not found in inventory"));
		return;
	}

	UWeaponData* WeaponData = Cast<UWeaponData>(GMR->GetItemDataById(Item->ItemId));
	if (!WeaponData) {
		UE_LOG(LogTemp, Warning, TEXT("OnRep_CurrentInventoryId: Weapon data not found for ItemId %d"), (int32)Item->ItemId);
		return;
	}
	FActorSpawnParameters Params;
	Params.Owner = GetOwner();
	Params.Instigator = Cast<APawn>(GetOwner());
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	AWeaponBase* NewWeapon = nullptr;
	if (WeaponData->WeaponType == EWeaponTypes::Firearm) {
		NewWeapon = GetWorld()->SpawnActor<AWeaponFirearm>(
			AWeaponFirearm::StaticClass(),
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			Params
		);
	}
	else {
		// Handle other weapon types (e.g., Melee)
	}

	if (!NewWeapon) {
        UE_LOG(LogTemp, Warning, TEXT("OnRep_CurrentInventoryId: Failed to spawn weapon actor"));
        return;
	}
	
	NewWeapon->InitFromData(WeaponData); // optional setup

    if (CurrentWeapon)
    {
        CurrentWeapon->Destroy();
    }
    CurrentWeapon = NewWeapon;
   
    EWeaponTypes WeaType = CurrentWeapon->GetWeaponType();

    FVector offset = FVector(0.f, 0.f, 0.f);
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

void UWeaponComponent::OnNewItemPickup(int32 NewInventoryId) {
    bool ShouldEquipNow = true;

    if (ShouldEquipNow) {
		EquipWeapon(NewInventoryId);
    }
}

EWeaponTypes UWeaponComponent::GetCurrentWeaponType() {
    if (CurrentWeapon) {
        return CurrentWeapon->GetWeaponType();
    }
    return EWeaponTypes::Unarmed;
}

void UWeaponComponent::DropWeapon() {
	UE_LOG(LogTemp, Warning, TEXT("DropWeapon called"));
    if (CurrentWeapon) {
        if (!GetOwner()->HasAuthority()) {
            ServerDropWeapon();
            return;
        }   
		HandleDropWeapon();
    }
}

void UWeaponComponent::ServerDropWeapon_Implementation() {
	HandleDropWeapon();
}

void UWeaponComponent::HandleDropWeapon() {
    if (CurrentWeapon) {
        CurrentWeapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
        CurrentWeapon->Destroy();
        // Throw it forward
        ABaseCharacter* Character = Cast<ABaseCharacter>(GetOwner());
        FVector ForwardVector = Character->GetActorForwardVector();
        FVector LaunchVelocity = ForwardVector * 400.f + FVector(0.f, 0.f, 100.f);
        //CurrentWeapon->WeaponMesh->AddImpulse(LaunchVelocity, NAME_None, true);

		// Spawn Pickup item
        APickupItem* Pickup = GetWorld()->SpawnActor<APickupItem>(
            APickupItem::StaticClass(),
            CurrentWeapon->GetActorLocation(),
            FRotator::ZeroRotator
		);

        if (Pickup) {
			FPickupData Data;
			Data.ItemId = CurrentWeapon->GetWeaponData()->Id;
			Data.Amount = 1;
			Data.Location = CurrentWeapon->GetActorLocation();

			Pickup->SetData(Data);
            Pickup->GetItemMesh()->AddImpulse(LaunchVelocity, NAME_None, true);
		}
        CurrentWeapon = nullptr;
    }
}

void UWeaponComponent::RequestFireStart() {
    if (!GetOwner()->HasAuthority()) {
        ServerStartFire();
        return;
    }
    HandleStartFire();
}

void UWeaponComponent::RequestFireStop() {
    if (!GetOwner()->HasAuthority()) {
        ServerStopFire();
        return;
	}
	HandleStopFire();
}

void UWeaponComponent::StartReload() {
    if (CurrentWeapon && !bIsReloading) {
        
    }
}

void UWeaponComponent::ServerStartFire_Implementation() {
	UE_LOG(LogTemp, Warning, TEXT("ServerStartFire called"));
	HandleStartFire();
}

void UWeaponComponent::ServerStopFire_Implementation() {
	HandleStopFire();
}

void UWeaponComponent::StartAiming() {
    bIsAiming = true;
}

bool UWeaponComponent::CanShoot() {
    ABaseCharacter* Character = Cast<ABaseCharacter>(GetOwner());
    if (Character->IsRunning()) {
		return false;
    }
    if (bIsReloading) {
        return false;
    }
    if (!CurrentWeapon) {
        return false;
    }
    if (CurrentWeapon->GetWeaponType() != EWeaponTypes::Firearm) {
        return false;
    }
    AWeaponFirearm* Firearm = Cast<AWeaponFirearm>(CurrentWeapon);
    if (Firearm) {
        return Firearm->CanFire();
    }
    return false;
}

void UWeaponComponent::HandleStartFire() {
    if (CanShoot()) {
        bIsFiring = true;
        float timeBetweenShots = 0.2f; // Example value, adjust as needed

        OnFire();
        GetOwner()->GetWorldTimerManager().SetTimer(FireTimerHandle, this, &UWeaponComponent::OnFire, timeBetweenShots, true);
    }
}

void UWeaponComponent::HandleStopFire() {
    if (bIsFiring) {
        GetOwner()->GetWorldTimerManager().ClearTimer(FireTimerHandle);
        bIsFiring = false;
	}
}

void UWeaponComponent::OnFire() {
    if (CurrentWeapon) {
        ABaseCharacter* Character = Cast<ABaseCharacter>(GetOwner());
        if (Character) {
            FVector CameraLocation;
            FRotator CameraRotation;
            Character->Controller->GetPlayerViewPoint(CameraLocation, CameraRotation);
            FVector ShotDirection = CameraRotation.Vector();

            FVector TraceEnd = CameraLocation + (ShotDirection * 10000.f);

            FHitResult Hit;
            FCollisionQueryParams Params;
            Params.AddIgnoredActor(Character);

            bool bHit = GetWorld()->LineTraceSingleByChannel(
                Hit,
                CameraLocation,
                TraceEnd,
                ECC_Visibility,
                Params
            );

            FVector TargetPoint = bHit ? Hit.ImpactPoint : TraceEnd;
           
            PlayEffectFire(TargetPoint);
            if (GetOwner()->HasAuthority()) // only server makes changes
            {
                MulticastPlayFireRifle(TargetPoint);
            }
        }
    }
}

void UWeaponComponent::PlayEffectFire(FVector TargetPoint) {
    // print log
	UE_LOG(LogTemp, Warning, TEXT("PlayEffectFire ccdalled"));

    if (!CurrentWeapon) {
        return;
    }
    
    if (CurrentWeapon->GetWeaponType() == EWeaponTypes::Firearm) {
        APlayerCharacter* Player = Cast<APlayerCharacter>(GetOwner());
        Player->PlayFireRifleMontage(TargetPoint);

        CurrentWeapon->OnFire(TargetPoint);
    }
}

void UWeaponComponent::MulticastPlayFireRifle_Implementation(FVector TargetPoint) {
    if (IsLocalControl()) {
        return;
    }
    PlayEffectFire(TargetPoint);
}

bool UWeaponComponent::IsLocalControl() {
    ACharacter* OwnerChar = Cast<ACharacter>(GetOwner());
    if (!OwnerChar) return false;

    // Skip local player
    if (OwnerChar->IsLocallyControlled())
    {
        return true;
    }

    return false;
}

bool UWeaponComponent::IsScopeEquipped()
{
    return bIsScopeEquipped;
}

void UWeaponComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(UWeaponComponent, CurrentInventoryId);
}