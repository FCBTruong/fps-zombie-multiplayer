// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/WeaponComponent.h"
#include "Characters/BaseCharacter.h"
#include "Weapons/WeaponDataManager.h"
#include "Weapons/WeaponFirearm.h"
#include "Characters/PlayerCharacter.h"
#include "Weapons/WeaponMelee.h"
#include "Weapons/WeaponThrowable.h"
#include "Kismet/GameplayStatics.h"
#include "Projectiles/AThrownProjectile.h"


// Sets default values for this component's properties
UWeaponComponent::UWeaponComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	bIsInitialized = false;
	SetIsReplicated(true);
}


// Called when the game starts
void UWeaponComponent::BeginPlay()
{
	Super::BeginPlay();

    GMR = GetWorld()->GetGameInstance()->GetSubsystem<UGameManager>();
	InventoryComp = GetOwner()->FindComponentByClass<UInventoryComponent>();
    Character = Cast<ABaseCharacter>(GetOwner());

	bIsInitialized = true;

    GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
        {
            InitState();
        });
}

void UWeaponComponent::InitState() {
    /*if (GetOwnerRole() == ROLE_Authority)
    {
        CurrentInventoryId = InventoryComp->GetMeleeId();
    }
    else
    {
        OnRep_CurrentInventoryId();
    }*/
}


// Called every frame
void UWeaponComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UWeaponComponent::EquipWeapon(int32 InventoryId)
{
    if (GetOwner()->GetLocalRole() < ROLE_Authority) {
        // Client
        ServerEquipWeapon(InventoryId);
        return;
	}
    else {
        // Server
        HandleEquipWeapon(InventoryId);
	}

}


void UWeaponComponent::ServerEquipWeapon_Implementation(int32 InventoryId)
{
	HandleEquipWeapon(InventoryId);
}

// Server function
void UWeaponComponent::HandleEquipWeapon(int32 InventoryId) {
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
    
    UWeaponData* WeaponData = Cast<UWeaponData>(GMR->GetItemDataById(Item->ItemId));
    FActorSpawnParameters Params;
    Params.Owner = GetOwner();
    Params.Instigator = Cast<APawn>(GetOwner());
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    
    if (CurrentWeapon) {
        CurrentWeapon->Destroy();
        CurrentWeapon = nullptr;
    }
    if (WeaponData->WeaponType == EWeaponTypes::Firearm) {
        CurrentWeapon = GetWorld()->SpawnActor<AWeaponFirearm>(
            AWeaponFirearm::StaticClass(),
            FVector::ZeroVector,
            FRotator::ZeroRotator,
            Params
        );
    }
    else if (WeaponData->WeaponType == EWeaponTypes::Melee) {
        // Handle Melee weapon spawning
        CurrentWeapon = GetWorld()->SpawnActor<AWeaponMelee>(
            FVector::ZeroVector,
            FRotator::ZeroRotator,
            Params
        );
    }
    else if (WeaponData->WeaponType == EWeaponTypes::Throwable) {
        CurrentWeapon = GetWorld()->SpawnActor<AWeaponThrowable>(
            FVector::ZeroVector,
            FRotator::ZeroRotator,
            Params
        );
    }

    EWeaponTypes WeaType = CurrentWeapon->GetWeaponType();
	CurrentWeapon->InitFromData(WeaponData);

    UpdateAttachLocationWeapon();

    // Play holster animation
    Character->PlayEquipWeaponAnimation(WeaType);

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


	// update speed based on weapon
    if (Character && CurrentWeapon) {
		// melee, rifle, pistol
        float NewSpeed = ABaseCharacter::NORMAL_WALK_SPEED;

        if (WeaType == EWeaponTypes::Firearm) {
            NewSpeed = ABaseCharacter::NORMAL_WALK_SPEED;
        }
        else if (WeaType == EWeaponTypes::Melee) {
            NewSpeed = ABaseCharacter::MELEE_WALK_SPEED;
        }
		Character->SetSpeedWalkCurrently(NewSpeed);
	}

    if (bIsAiming) {
		bIsAiming = false;
    }
}



void UWeaponComponent::OnUpdateCurrentWeaponData() {
    
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
	if (!CurrentWeapon) {
        UE_LOG(LogTemp, Warning, TEXT("HandleDropWeapon: No weapon to drop"));
        return;
	}
    // Can not drop meleee
    if (CurrentWeapon->GetWeaponType() == EWeaponTypes::Melee) {
        UE_LOG(LogTemp, Warning, TEXT("HandleDropWeapon: Can not drop melee weapon"));
        return;
	}

	// Remove from inventory
    if (InventoryComp) {
        //InventoryComp->RemoveItemByInventoryId(CurrentInventoryId);
    }

	// spawn new pickup item on map
	FVector DropPoint = GetOwner()->GetActorLocation() + GetOwner()->GetActorForwardVector() * 300.f + FVector(0.f, 0.f, 100.f);
    FPickupData Data;
	Data.Location = DropPoint;
	Data.Amount = 1;
	Data.ItemId = CurrentWeapon->GetWeaponData()->Id;
	Data.Id = GMR->GetNextItemOnMapId();
    GMR->OnNewItemDataSpawned({ Data });

	OnUpdateCurrentWeaponData();

	MulticastDropWeapon(Data);

	CurrentWeapon->Destroy();
	CurrentWeapon = nullptr;
    EquipSlot(FGameConstants::SLOT_MELEE);
}

void UWeaponComponent::MulticastDropWeapon_Implementation(FPickupData Data) {
    // Throw it forward
    FVector ForwardVector = Character->GetActorForwardVector();
    FVector LaunchVelocity = ForwardVector * 300.f + FVector(0.f, 0.f, 100.f);
    //CurrentWeapon->WeaponMesh->AddImpulse(LaunchVelocity, NAME_None, true);

    // Spawn Pickup item
    APickupItem* Pickup = GetWorld()->SpawnActor<APickupItem>(
        APickupItem::StaticClass(),
        GetOwner()->GetActorLocation(),
        FRotator::ZeroRotator
    );

    if (Pickup) {
        Pickup->SetData(Data);
        Pickup->GetItemMesh()->AddImpulse(LaunchVelocity, NAME_None, true);
        Pickup->GetItemMesh()->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
        Pickup->GetItemMesh()->SetEnableGravity(true);
        Pickup->GetItemMesh()->SetLinearDamping(0.8f);
        Pickup->GetItemMesh()->SetAngularDamping(0.8f);
        Pickup->GetItemMesh()->SetPhysicsMaxAngularVelocityInDegrees(720.f);

		// add pickup data to game state due to server will not update for clients
		GMR->OnNewItemDataSpawned({ Data });
		GMR->OnNewItemNodeSpawned(Pickup, Data.Id);
    }
}

void UWeaponComponent::StartReload() {
    if (!bIsReloading) {
        if (GetOwner()->GetLocalRole() < ROLE_Authority) {
            ServerReload();
        }
        else {
            HandleReload();
		}
    }
}

void UWeaponComponent::StartAiming() {
    bIsAiming = true;
}

// This function only apply for firearms
bool UWeaponComponent::CanShoot() {
    if (Character->IsRunning()) {
		return false;
    }
    if (bIsReloading) {
        return false;
    }
    if (!CurrentWeapon) {
        return false;
    }
    if (Character->IsCloseToWall()) {
        return false;
	}
    // check has ammo left
    if (!CurrentWeapon->HasAmmoInClip()) {
        return false;
    }
    return true;
}

void UWeaponComponent::OnLeftClickStart() {
    if (CurrentWeapon == nullptr) {
        UE_LOG(LogTemp, Warning, TEXT("HandleStartFire: No weapon equipped"));
        return;
	}

    // is fire arm
    if (CurrentWeapon->GetWeaponType() == EWeaponTypes::Firearm) {
        if (CanShoot()) {
            bIsFiring = true;
            float timeBetweenShots = 0.2f; // Example value, adjust as needed

            OnFire();
            GetOwner()->GetWorldTimerManager().SetTimer(FireTimerHandle, this, &UWeaponComponent::OnFire, timeBetweenShots, true);
        }
        if (CurrentWeapon) {
            if (AWeaponFirearm* Firearm = Cast<AWeaponFirearm>(CurrentWeapon))
            {
                if (!Firearm->HasAmmoInClip())
                {
					Firearm->PlayOutOfAmmoSound();
                }
			}
        }
    }
    else if (CurrentWeapon->GetWeaponType() == EWeaponTypes::Melee) {
         ServerDoMeleeAttack(0);
    }
    else if (CurrentWeapon->GetWeaponType() == EWeaponTypes::Throwable) {
        if (!bIsPriming) {
            DrawProjectileCurve();
            ServerSetIsPriming(true);
        }
	}
}

void UWeaponComponent::OnLeftClickRelease() {
    if (bIsFiring) {
        GetOwner()->GetWorldTimerManager().ClearTimer(FireTimerHandle);
        bIsFiring = false;
	}

    if (CurrentWeapon && CurrentWeapon->GetWeaponType() == EWeaponTypes::Throwable) {
        // Throw the grenade
        if (!bIsPriming) {
            return; // not priming, ignore
        }
        
        ServerThrow(GetVelocityGrenade());
	}
}

void UWeaponComponent::ServerThrow_Implementation(FVector LaunchVelocity) {
	if (!bIsPriming) {
		return; // not priming, ignore
	}

    if (bIsThrowing) {
		return; // already throwing
    }
	bIsThrowing = true;
    // Logic throw, move object, and explode
    // Because on server, there's no current weapon, so we need to handle this
	// gen object throwable
    FVector StartPos = Character->GetThrowableLocation();
    //StartPos += Character->GetActorForwardVector() * 10.f; // avoid collision                       // raise a bit if needed


    AAThrownProjectile* ThrownProj = GetWorld()->SpawnActor<AAThrownProjectile>(
        AAThrownProjectile::StaticClass(),
        StartPos,
        FRotator::ZeroRotator
	);
    if (ThrownProj) {
		ThrownProj->SetOwner(GetOwner());
        ThrownProj->InitFromData(CurrentWeapon->GetWeaponData());
        ThrownProj->LaunchProjectile(LaunchVelocity, Character);
	}

    bIsPriming = false;
    FTimerHandle TimerHandle_FinishThrow;
    GetWorld()->GetTimerManager().SetTimer(
        TimerHandle_FinishThrow,
        this,
        &UWeaponComponent::OnFinishedThrow,
        0.5f,
        false
    );

    MulticastThrowAction(LaunchVelocity);
}

void UWeaponComponent::OnFinishedThrow() {
	bIsThrowing = false;
	EquipSlot(FGameConstants::SLOT_MELEE);
}

void UWeaponComponent::MulticastThrowAction_Implementation(FVector LaunchVelocity) {
	UE_LOG(LogTemp, Warning, TEXT("MulticastThrowAction called"));
    if (TrajectoryPreviewRef) {
        TrajectoryPreviewRef->Destroy();
        TrajectoryPreviewRef = nullptr;
    }

    if (CurrentWeapon) {
		CurrentWeapon->Destroy();
		CurrentWeapon = nullptr;
    }
    Character->PlayThrowNadeMontage();
    GetOwner()->GetWorldTimerManager().ClearTimer(ThrowProjectileTimer);
}

// Client function, Client detects hit point and sends to server
void UWeaponComponent::OnFire() {
	UE_LOG(LogTemp, Warning, TEXT("OnFire: called"));
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
           
		ServerOnFire(TargetPoint);
    }
}

void UWeaponComponent::ServerOnFire_Implementation(FVector TargetPoint) {
	HandleOnFire(TargetPoint);
}

void UWeaponComponent::ServerDoMeleeAttack_Implementation(int AttackIdx) {
    if (bIsMeleeAttacking) {
        return; // already attacking
	}
	// Check if can attack
    if (!Character) {
        return;
    }
   /* if (Character->IsCloseToWall()) {
        return;
    }*/
	bIsMeleeAttacking = true;
	Character->PlayMeleeAttackAnimation(AttackIdx);
	MulticastDoMeleeAttack(AttackIdx);
}

void UWeaponComponent::MulticastDoMeleeAttack_Implementation(int AttackIdx) {
    Character->PlayMeleeAttackAnimation(AttackIdx);
}


// Server function
void UWeaponComponent::HandleOnFire(FVector TargetPoint) {
    if (GetOwner()->HasAuthority()) // only server makes changes
    {
        if (!Character) {
            return;
        }

        // check current weapon
        if (!CurrentWeapon) {
			UE_LOG(LogTemp, Warning, TEXT("OnFire: Server no current weapon"));
            return;
        }
		if (!CanShoot()) {
            UE_LOG(LogTemp, Warning, TEXT("OnFire: Server can not shoot now"));
            return;
		}
		// decrease ammo
        if (AWeaponFirearm* Firearm = Cast<AWeaponFirearm>(CurrentWeapon))
        {
            Firearm->ConsumeAmmo(1);
        }

        // start from head or eyes
        FVector Start = Character->GetPawnViewLocation();
        FVector End = TargetPoint;

        FHitResult Hit;
        FCollisionQueryParams Params;
        Params.AddIgnoredActor(Character);

        bool bHit = GetWorld()->LineTraceSingleByChannel(
            Hit, Start, End, ECC_Visibility, Params
        );

		float Damage = 25.f; // Example damage value
        if (bHit && Damage > 0.f)
        {
			UE_LOG(LogTemp, Warning, TEXT("OnFire: Server applying damage to %s"), *Hit.GetActor()->GetName());
            float ActualDamage = UGameplayStatics::ApplyDamage(
                Hit.GetActor(), Damage, Character->GetController(),
                Character, UDamageType::StaticClass()
            );
			UE_LOG(LogTemp, Warning, TEXT("OnFire: Server applied damage: %f"), ActualDamage);
        }

        UE_LOG(LogTemp, Warning, TEXT("OnFire: Server calling MulticastPlayFireRifle"));
        MulticastPlayFireRifle(TargetPoint);
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
    if (GetOwner()->HasAuthority()) 
    {
		UE_LOG(LogTemp, Warning, TEXT("MulticastPlayFireRifle: Called on server, ignoring"));
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
    if (CurrentWeapon)
    {
    
		UWeaponData* WeaponData = CurrentWeapon->GetWeaponData();
        if (WeaponData && WeaponData->WeaponType == EWeaponTypes::Firearm && WeaponData->WeaponSubType == EWeaponSubTypes::Sniper)
        {
            return true;
        }
	}
    return false;
}

void UWeaponComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UWeaponComponent, bIsPriming);
}

// Clients press 1, 2, 3 to equip weapon in that slot
void UWeaponComponent::EquipSlot(int32 SlotIndex)
{
    if (bIsPriming) {
        return; // can not change weapon while priming
    }
    int32 InventoryId = FGameConstants::INVENTORY_ID_NONE;

    if (SlotIndex == FGameConstants::SLOT_THROWABLE) {
        InventoryId = InventoryComp->GetFirstInventoryIdByType(EWeaponTypes::Throwable);
    }
    else if (SlotIndex == FGameConstants::SLOT_LONG_GUN_1) {
        InventoryId = GetLongGunInvenId();
    }
    else if (SlotIndex == FGameConstants::SLOT_MELEE) {
        InventoryId = GetMeleeInvenId();
    }
    else if (SlotIndex == FGameConstants::SLOT_PISTOL) {
        InventoryId = GetSideArmInvenId();
    }
	UE_LOG(LogTemp, Warning, TEXT("EquipSlot called for slot %d, found InventoryId %d"), SlotIndex, InventoryId);

    if (InventoryId != FGameConstants::INVENTORY_ID_NONE)
    {
        EquipWeapon(InventoryId);
    }
}

void UWeaponComponent::DrawProjectileCurve()
{
	UE_LOG(LogTemp, Warning, TEXT("DrawProjectileCurve called"));
    GetOwner()->GetWorldTimerManager().SetTimer(
        ThrowProjectileTimer,
        this,
        &UWeaponComponent::UpdateProjectileCurve,
        0.03f,
        true
    );


    TrajectoryPreviewRef = GetWorld()->SpawnActor<ATrajectoryPreview>(
        FVector::ZeroVector,
        FRotator::ZeroRotator
    );
    if (!Character->ThrowSpline) {
		UE_LOG(LogTemp, Warning, TEXT("DrawProjectileCurve: SplineRef is invalid"));
    }
    TrajectoryPreviewRef->SplineRef = Character->ThrowSpline;
}

void UWeaponComponent::UpdateProjectileCurve()
{
    if (!Character || !Character->ThrowSpline)
    {
        return;
	}
	FVector StartPos = Character->GetThrowableLocation();
    FVector LaunchVelocity = GetVelocityGrenade();

    FPredictProjectilePathParams Params;
    Params.StartLocation = StartPos;
    Params.LaunchVelocity = LaunchVelocity;
    Params.ProjectileRadius = 0.0f;
    Params.TraceChannel = ECC_WorldDynamic;
    Params.bTraceComplex = true;
    Params.MaxSimTime = 2.0f;
    Params.SimFrequency = 15.0f;
    Params.DrawDebugTime = 0.0f;
    Params.DrawDebugType = EDrawDebugTrace::None;
    Params.OverrideGravityZ = 0.0f;
    Params.ActorsToIgnore.Add(Character);

    FPredictProjectilePathResult Result;
    UGameplayStatics::PredictProjectilePath(this, Params, Result);

    Character->ThrowSpline->ClearSplinePoints();

    for (const FPredictProjectilePathPointData& Point : Result.PathData)
    {
        Character->ThrowSpline->AddSplinePoint(Point.Location, ESplineCoordinateSpace::World);
    }
}

FVector UWeaponComponent::GetVelocityGrenade() const
{
    FRotator ControlRot = Character->GetControlRotation();

    // Add pitch offset (ThrowAngle is a float variable you define)
    FRotator ThrowRot = FRotator(ControlRot.Pitch + ThrowAngle, ControlRot.Yaw, ControlRot.Roll);

    // Convert to forward vector
    FVector Forward = ThrowRot.Vector();

    // Multiply by initial grenade speed (float variable)
    return Forward * GrenadeInitSpeed;
}


void UWeaponComponent::OnRep_IsPriming()
{
    // Handle changes when bIsPriming is replicated
    if (bIsPriming)
    {
        // Start priming effects
        UE_LOG(LogTemp, Warning, TEXT("OnRep_IsPriming: Started priming"));
		Character->PlayHoldNadeMontage();
    }
    else
    {
        // Stop priming effects
        UE_LOG(LogTemp, Warning, TEXT("OnRep_IsPriming: Stopped priming"));
    }
}


void UWeaponComponent::ServerSetIsPriming_Implementation(bool bNewIsPriming)
{
	UE_LOG(LogTemp, Warning, TEXT("ServerSetIsPriming called with %s"), bIsPriming ? TEXT("true") : TEXT("false"));
    if (bIsPriming == bNewIsPriming) {
        return; // no change
	}
    bIsPriming = bNewIsPriming;
    OnRep_IsPriming();
}

int UWeaponComponent::GetLongGunInvenId() {
    if (LongGunInventoryId == FGameConstants::INVENTORY_ID_NONE) {
        // try find weapon firearm, and long gun

        for (const FInventoryItem& Item : InventoryComp->GetItems()) {
            if (GMR) {
                const UItemData* ItemData = GMR->GetItemDataById(Item.ItemId);
                UE_LOG(LogTemp, Warning, TEXT("Checking item InventoryId %d for main"), Item.InventoryId);
                if (ItemData && ItemData->IsA(UWeaponData::StaticClass())) {
                    // log debug
                    UE_LOG(LogTemp, Warning, TEXT("ItemData found for InventoryId %d, checking weapon type."), Item.InventoryId);
                    const UWeaponData* WeaponData = Cast<UWeaponData>(ItemData);
                    if (WeaponData && WeaponData->WeaponType == EWeaponTypes::Firearm &&
                        (WeaponData->WeaponSubType == EWeaponSubTypes::Rifle
							|| WeaponData->WeaponSubType == EWeaponSubTypes::Sniper
                            )) {
                        UE_LOG(LogTemp, Warning, TEXT("Assigning rifle weapon InventoryId %d to longun slot."), Item.InventoryId);
                        LongGunInventoryId = Item.InventoryId;
                        break;
                    }
                }
            }
        }
    }
    else {
        if (!InventoryComp->CheckExistItem(LongGunInventoryId)) {
            LongGunInventoryId = FGameConstants::INVENTORY_ID_NONE;
        }
    }
    return LongGunInventoryId;
}

int UWeaponComponent::GetSideArmInvenId() {
    return 0;
}

int UWeaponComponent::GetMeleeInvenId() {
    if (MeleeInventoryId == FGameConstants::INVENTORY_ID_NONE) {
        // try find weapon firearm, and long gun

        for (const FInventoryItem& Item : InventoryComp->GetItems()) {
            if (GMR) {
                const UItemData* ItemData = GMR->GetItemDataById(Item.ItemId);
                UE_LOG(LogTemp, Warning, TEXT("Checking item InventoryId %d for main"), Item.InventoryId);
                if (ItemData && ItemData->IsA(UWeaponData::StaticClass())) {
                    // log debug
                    UE_LOG(LogTemp, Warning, TEXT("ItemData found for InventoryId %d, checking weapon type."), Item.InventoryId);
                    const UWeaponData* WeaponData = Cast<UWeaponData>(ItemData);
                    if (WeaponData && WeaponData->WeaponType == EWeaponTypes::Melee) {
                        UE_LOG(LogTemp, Warning, TEXT("Assigning rifle weapon InventoryId %d to longun slot."), Item.InventoryId);
                        MeleeInventoryId = Item.InventoryId;
                        break;
                    }
                }
            }
        }
    }
    return MeleeInventoryId;
}

void UWeaponComponent::UpdateAttachLocationWeapon() {
    if (!CurrentWeapon || !Character) {
        return;
    }

    FVector offset = FVector(0.f, 0.f, 0.f);
	FVector offsetRot = FVector(0.f, 0.f, 0.f);
    FString SocketName = "ik_hand_gun";
    bool bIsFPS = Character->IsFpsViewMode();
    if (!CurrentWeapon->GetWeaponData()) {
		UE_LOG(LogTemp, Warning, TEXT("UpdateAttachLocationWeapon: No weapon data found"));
        return;
    }
    EWeaponTypes WeaType = CurrentWeapon->GetWeaponType();
    if (WeaType == EWeaponTypes::Firearm) {
        if (bIsFPS) {
            SocketName = "ik_hand_gun";
            offset = CurrentWeapon->GetWeaponData()->EquippedOffsetFps;
            offsetRot = CurrentWeapon->GetWeaponData()->EquippedOffsetRotationFps;
        }
        else {
            SocketName = "weapon_socket";
            offset = CurrentWeapon->GetWeaponData()->EquippedOffset;
            offsetRot = CurrentWeapon->GetWeaponData()->EquippedOffsetRotation;
        }
    }
    else if (WeaType == EWeaponTypes::Melee) {
        SocketName = "melee_socket";
    }
    else if (WeaType == EWeaponTypes::Throwable) {
        SocketName = "throwable_socket";
    }

    CurrentWeapon->AttachToComponent(Character->GetCurrentMesh(),
        FAttachmentTransformRules::SnapToTargetNotIncludingScale,
        FName(SocketName)
    );
    USceneComponent* Root = CurrentWeapon->GetRootComponent();

	Root->SetRelativeLocationAndRotation(offset, FRotator::MakeFromEuler(offsetRot));
}

bool UWeaponComponent::CanWeaponAim() {
    if (CurrentWeapon == nullptr) {
        return false;
    }
    if (CurrentWeapon->GetWeaponType() != EWeaponTypes::Firearm) {
        return false;
    }
    return true;
}

void UWeaponComponent::PerformMeleeAttack(int AttackIdx)
{
	UE_LOG(LogTemp, Warning, TEXT("PerformMeleeAttack called"));
    if (!Character) return;
    FVector Start = Character->GetActorLocation() + FVector(0, 0, 50);
    FVector End = Start + Character->GetActorForwardVector() * 150;
    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(Character);

	if (Character->HasAuthority()) {
		bIsMeleeAttacking = false;
	}

    if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Pawn, Params))
    {
        if (AActor* Target = Hit.GetActor())
        {
			UE_LOG(LogTemp, Warning, TEXT("PerformMeleeAttack: Hit actor %s"), *Target->GetName());
            if (Character->MeleeHitDecal) {
				UE_LOG(LogTemp, Warning, TEXT("PerformMeleeAttack: Spawning decal at hit location"));
                FVector SpawnLoc = Hit.ImpactPoint + Hit.ImpactNormal * 2.0f;
                FRotator DecalRot = Hit.ImpactNormal.Rotation();
                UGameplayStatics::SpawnDecalAtLocation(
                    GetWorld(),
                    Character->MeleeHitDecal,
                    FVector(10.f, 10.f, 10.f), // XYZ = size, not just X
                    SpawnLoc,
                    DecalRot,
                    10.0f
                );

            }

            UGameplayStatics::ApplyDamage(Target, 10, Character->GetController(), Character, UDamageType::StaticClass());
        }
    }
}


void UWeaponComponent::OnRep_CurrentWeapon()
{
    // Handle changes when CurrentWeapon is replicated
    UE_LOG(LogTemp, Warning, TEXT("OnRep_CurrentWeapon called"));
    UpdateAttachLocationWeapon();

	// Play equip animation
    if (Character && CurrentWeapon) {
        EWeaponTypes WeaType = CurrentWeapon->GetWeaponType();
        Character->PlayEquipWeaponAnimation(WeaType);
	}
}

void UWeaponComponent::ServerReload_Implementation()
{
	HandleReload();
}

void UWeaponComponent::HandleReload()
{
    UE_LOG(LogTemp, Warning, TEXT("OnEquipWeaponFinished called"));
    if (bIsReloading) {
        return; // already reloading
    }
    // check can reload
    if (!CurrentWeapon) {
        return;
	}
    if (CurrentWeapon->GetWeaponType() != EWeaponTypes::Firearm) {
		return; // only firearm can reload
	}

    bIsReloading = true;
    
	MulticastReload();
    FTimerHandle TimerHandle_FinishReload;
    GetWorld()->GetTimerManager().SetTimer(
        TimerHandle_FinishReload,
        this,
        &UWeaponComponent::OnFinishedReload,
        2.0f,
        false
    );
}

void UWeaponComponent::MulticastReload_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("MulticastReload called"));
    if (Character) {
        Character->PlayReloadMontage();
    }

    if (CurrentWeapon) {
        if (AWeaponFirearm* Firearm = Cast<AWeaponFirearm>(CurrentWeapon))
        {
            Firearm->PlayReloadSound();
		}
	}
}

void UWeaponComponent::OnFinishedReload()
{
    bIsReloading = false;
    if (AWeaponFirearm* Firearm = Cast<AWeaponFirearm>(CurrentWeapon))
    {
        Firearm->SetCurrentAmmo(50);
    }
}

void UWeaponComponent::OnNotifyGrabMag() {
	UE_LOG(LogTemp, Warning, TEXT("OnNotifyGrabMag called"));
    if (CurrentWeapon) {
        if (AWeaponFirearm* Firearm = Cast<AWeaponFirearm>(CurrentWeapon))
        {
            if (Firearm->MagMesh == nullptr) {
                UE_LOG(LogTemp, Warning, TEXT("OnNotifyInsertMag: MagMesh is null"));
                return;
            }
           /* Firearm->PlayGrabMagSound();*/
			UE_LOG(LogTemp, Warning, TEXT("OnNotifyGrabMag: Attaching mag to hand_r"));
            Firearm->MagMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
            if (Character->IsFpsViewMode()) {
                Firearm->MagMesh->AttachToComponent(
                    Character->GetCurrentMesh(),
                    FAttachmentTransformRules::KeepWorldTransform,
                    FName("hand_l")
                );
			}
            else {
                Firearm->MagMesh->AttachToComponent(
                    Character->GetCurrentMesh(),
                    FAttachmentTransformRules::KeepRelativeTransform,
                    FName("hand_l")
                );

                // Snap position only
                FVector TargetLoc = Character->GetCurrentMesh()->GetSocketLocation("hand_l");
                Firearm->MagMesh->SetWorldLocation(TargetLoc);
            }
        }
	}
}

void UWeaponComponent::OnNotifyInsertMag() {
	UE_LOG(LogTemp, Warning, TEXT("OnNotifyInsertMag called"));
    if (CurrentWeapon) {
        if (AWeaponFirearm* Firearm = Cast<AWeaponFirearm>(CurrentWeapon))
        {
			if (Firearm->MagMesh == nullptr) {
				UE_LOG(LogTemp, Warning, TEXT("OnNotifyInsertMag: MagMesh is null"));
				return;
			}
            Firearm->MagMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
            Firearm->AttachMagToDefault();
        }
    }
}