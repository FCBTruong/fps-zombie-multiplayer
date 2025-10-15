// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/WeaponComponent.h"
#include "Characters/BaseCharacter.h"
#include "Weapons/WeaponDataManager.h"
#include "Weapons/WeaponFirearm.h"
#include "Characters/PlayerCharacter.h"
#include "Weapons/WeaponMelee.h"


// Sets default values for this component's properties
UWeaponComponent::UWeaponComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
    CurrentInventoryId = FGameConstants::INVENTORY_ID_NONE;
	bIsInitialized = false;
	SetIsReplicated(true);
}


// Called when the game starts
void UWeaponComponent::BeginPlay()
{
	Super::BeginPlay();

    GMR = GetWorld()->GetGameInstance()->GetSubsystem<UGameManager>();
	InventoryComp = GetOwner()->FindComponentByClass<UInventoryComponent>();

	bIsInitialized = true;
    EquipSlot(FGameConstants::SLOT_MELEE);

    GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
        {
            InitState();
        });
}

void UWeaponComponent::InitState() {
    if (GetOwnerRole() == ROLE_Authority)
    {

    }
    else
    {
        OnRep_CurrentInventoryId();
    }
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
    if (InventoryId == CurrentInventoryId) {
		return; // already equipped
    }
	// check if user has this item in inventory
    if (!InventoryComp) {
        UE_LOG(LogTemp, Warning, TEXT("ServerEquipWeapon_Implementation: Inventory component not found"));
        return;
	}
	FInventoryItem* Item = InventoryComp->GetItemByInventoryId(InventoryId);
    if (!Item) {
        UE_LOG(LogTemp, Warning, TEXT("ServerEquipWeapon_Implementation: Item with InventoryId %d not found in inventory"), InventoryId);
        return;
	}

	UE_LOG(LogTemp, Warning, TEXT("ServerEquipWeapon_Implementation called withaaaa InventoryId: %d"), InventoryId);
    CurrentInventoryId = InventoryId;
    OnUpdateCurrentWeaponData();
}

void UWeaponComponent::OnUpdateCurrentWeaponData() {
    if (CurrentInventoryId == FGameConstants::INVENTORY_ID_NONE) {
        CurrentWeaponData.WeaponData = nullptr;
        return;
	}
	FInventoryItem* Item = InventoryComp->GetItemByInventoryId(CurrentInventoryId);
	UWeaponData* WeaponData = Cast<UWeaponData>(GMR->GetItemDataById(Item->ItemId));
    CurrentWeaponData.WeaponData = WeaponData;
}

// Client function, handles weapon spawning and attaching
void UWeaponComponent::OnRep_CurrentInventoryId()
{
    if (!bIsInitialized) {
		UE_LOG(LogTemp, Warning, TEXT("OnRep_CurrentInventoryId called before initialization"));
		return;
	}
    OnUpdateCurrentWeaponData();
	// Gen new Weapon based on InventoryId
    if (CurrentInventoryId == FGameConstants::INVENTORY_ID_NONE) {
		UE_LOG(LogTemp, Warning, TEXT("OnRep_CurrentInventoryId: Invalid InventoryId"));
        return;
	}

	if (!InventoryComp) {
        UE_LOG(LogTemp, Warning, TEXT("OnRep_CurrentInventoryId: Inventory component not found"));
        return;
	}
	
	// debug ,print size inventory
	int32 x = InventoryComp->GetItemCount();
    UE_LOG(LogTemp, Warning, TEXT("OnRep_CurrentInventoryId: Inventory has %d items"), x);

	UWeaponData* WeaponData = CurrentWeaponData.WeaponData;
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
	else if (WeaponData->WeaponType == EWeaponTypes::Melee) {
        // Handle Melee weapon spawning
        NewWeapon = GetWorld()->SpawnActor<AWeaponMelee>(
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
    ABaseCharacter* Character = Cast<ABaseCharacter>(GetOwner());
    bool bIsFPS = Character->IsFpsViewMode();
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

    CurrentWeapon->AttachToComponent(Character->GetCurrentMesh(),
        FAttachmentTransformRules::SnapToTargetNotIncludingScale,
        FName(SocketName)
    );

    CurrentWeapon->SetActorRelativeLocation(offset);
	CurrentWeapon->SetActorHiddenInGame(false);

    // debug log, print socket name, is hideen, current fps
    UE_LOG(LogTemp, Warning, TEXT("Equipped weapon %s | Hidden: %d | FPS: %d | Socket: %s"),
        *CurrentWeapon->GetName(),
        CurrentWeapon->IsHidden(),
        bIsFPS,
        *SocketName);
    if (UWorld* World = GetWorld())
    {
        UE_LOG(LogTemp, Warning, TEXT("Worldxxx: %s | Scene: %s"),
            *World->GetName(),
            *World->GetMapName());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No valid world for %s"), *GetName());
    }

}

void UWeaponComponent::OnNewItemPickup(int32 NewInventoryId) {
    bool ShouldEquipNow = true;

    if (ShouldEquipNow) {
		EquipWeapon(NewInventoryId);
    }
}

EWeaponTypes UWeaponComponent::GetCurrentWeaponType() {
    if (CurrentWeaponData.WeaponData) {
        return CurrentWeaponData.WeaponData->WeaponType;
	}
	return EWeaponTypes::Unarmed;
}

void UWeaponComponent::DropWeapon() {
	UE_LOG(LogTemp, Warning, TEXT("DropWeapon called"));
    if (CurrentWeaponData.WeaponData) {
        if (!GetOwner()->HasAuthority()) {
            ServerDropWeapon();
        }
        else {
            HandleDropWeapon();
        }
    }
}

void UWeaponComponent::ServerDropWeapon_Implementation() {
	HandleDropWeapon();
}


// Server function
void UWeaponComponent::HandleDropWeapon() {
	// Remove from inventory
    if (InventoryComp) {
        InventoryComp->RemoveItemByInventoryId(CurrentInventoryId);
    }

	// spawn new pickup item on map
	FVector DropPoint = GetOwner()->GetActorLocation() + GetOwner()->GetActorForwardVector() * 300.f + FVector(0.f, 0.f, 100.f);
    FPickupData Data;
	Data.Location = DropPoint;
	Data.Amount = 1;
	Data.ItemId = CurrentWeaponData.WeaponData->Id;
	Data.Id = GMR->GetNextItemOnMapId();
    GMR->OnNewItemDataSpawned({ Data });

    // Detach and spawn pickup
    CurrentInventoryId = FGameConstants::INVENTORY_ID_NONE;
	OnUpdateCurrentWeaponData();

	MulticastDropWeapon(Data.Id, DropPoint);
}

void UWeaponComponent::MulticastDropWeapon_Implementation(int32 OnMapId, FVector DropPoint) {
    if (CurrentWeapon) {
        CurrentWeapon->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

        // Throw it forward
        ABaseCharacter* Character = Cast<ABaseCharacter>(GetOwner());
        FVector ForwardVector = Character->GetActorForwardVector();
        FVector LaunchVelocity = ForwardVector * 300.f + FVector(0.f, 0.f, 100.f);
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
			Data.Id = OnMapId;

            Pickup->SetData(Data);
            Pickup->GetItemMesh()->AddImpulse(LaunchVelocity, NAME_None, true);

			// add pickup data to game state due to server will not update for clients
			GMR->OnNewItemDataSpawned({ Data });
			GMR->OnNewItemNodeSpawned(Pickup, OnMapId);
        }
        CurrentWeapon->Destroy();
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
    if (!bIsReloading) {
        
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
    if (CurrentInventoryId == FGameConstants::INVENTORY_ID_NONE) {
        return false;
    }
    return true;
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

// Server function
void UWeaponComponent::OnFire() {
	UE_LOG(LogTemp, Warning, TEXT("OnFire: called"));
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
           
        /*PlayEffectFire(TargetPoint);*/
        if (GetOwner()->HasAuthority()) // only server makes changes
        {
			UE_LOG(LogTemp, Warning, TEXT("OnFire: Server calling MulticastPlayFireRifle"));
            MulticastPlayFireRifle(TargetPoint);
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

// Clients press 1, 2, 3 to equip weapon in that slot
void UWeaponComponent::EquipSlot(int32 SlotIndex)
{
    int32 InventoryId = InventoryComp->GetInventoryIdBySlot(SlotIndex);
	UE_LOG(LogTemp, Warning, TEXT("EquipSlot called for slot %d, found InventoryId %d"), SlotIndex, InventoryId);

    if (InventoryId != FGameConstants::INVENTORY_ID_NONE)
    {
        EquipWeapon(InventoryId);
    }
}
