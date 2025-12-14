// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/WeaponComponent.h"
#include "Characters/BaseCharacter.h"
#include "Weapons/WeaponDataManager.h"
#include "Weapons/WeaponFirearm.h"
#include "Weapons/WeaponMelee.h"
#include "Weapons/WeaponThrowable.h"
#include "Kismet/GameplayStatics.h"
#include "Projectiles/ThrownProjectile.h"
#include "Projectiles/ThrownProjectileFrag.h"
#include "Projectiles/ThrownProjectileSmoke.h"
#include "Projectiles/ThrownProjectileStun.h"
#include "Projectiles/ThrownProjectileIncendiary.h"
#include "Damage/MyDamageType.h"
#include "GameFramework/DamageType.h"
#include "Engine/EngineTypes.h"
#include "Damage/MyPointDamageEvent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Game/SpikeMode.h"
#include "Game/ShooterGameState.h"
#include "Engine/DecalActor.h"
#include "Components/DecalComponent.h"
#include "Game/ActorManager.h"
#include <Engine/TriggerBox.h>
#include "Components/BoxComponent.h"
#include <Kismet/KismetMathLibrary.h>
#include "Components/CapsuleComponent.h"

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
    UE_LOG(LogTemp, Warning, TEXT("WeaponComponent: BeginPlay"));
	Super::BeginPlay();

    Character = Cast<ABaseCharacter>(GetOwner());

	InventoryComp = GetOwner()->FindComponentByClass<UInventoryComponent>();

	bIsInitialized = true;

    GetWorld()->GetTimerManager().SetTimerForNextTick([this]()
        {
            InitState();
        });
}

void UWeaponComponent::InitState() {
    if (GetOwner()->HasAuthority()) {
        MeleeState.ItemId = EItemId::MELEE_KNIFE_BASIC;
        PistolState.ItemId = EItemId::PISTOL_PL_14;
		AutoEquipBestWeapon();

        UWeaponData* PistolData = UGameManager::Get(GetWorld())->GetWeaponDataById(EItemId::PISTOL_PL_14);
        PistolState.AmmoInClip = PistolData ? PistolData->MaxAmmoInClip : 0;
        PistolState.AmmoReserve = PistolData ? PistolData->MaxAmmoInClip * 2 : 0;
    }
}


// Called every frame
void UWeaponComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UWeaponComponent::EquipWeapon(EItemId ItemId)
{
    if (Character && !Character->IsAlive()) return;

    if (GetOwner()->GetLocalRole() < ROLE_Authority) {
        // Client
        ServerEquipWeapon(ItemId);
        return;
	}
    else {
        // Server
        HandleEquipWeapon(ItemId);
	}
}


void UWeaponComponent::ServerEquipWeapon_Implementation(EItemId ItemId)
{
	HandleEquipWeapon(ItemId);
}

// Server function
void UWeaponComponent::HandleEquipWeapon(EItemId ItemId) {
	if (ItemId == EItemId::NONE) {
        UE_LOG(LogTemp, Warning, TEXT("HandleEquipWeapon: Invalid ItemId NONE"));
        return;
	}

    if (!Character || !Character->IsAlive()) return;

	if (CurrentWeaponId == ItemId) 
    {
		UE_LOG(LogTemp, Warning, TEXT("HandleEquipWeapon: Weapon %d is "), (int32)ItemId);
        // already equipped
        return;
	}
    if(!UGameManager::Get(GetWorld())) {
        UE_LOG(LogTemp, Warning, TEXT("HandleEquipWeapon: GameManager is null"));
		return; 
	}

    UWeaponData* WeaponConf = UGameManager::Get(GetWorld())->GetWeaponDataById(ItemId);
    if (!WeaponConf) {
        UE_LOG(LogTemp, Warning, TEXT("HandleEquipWeapon: No weapon data found for %d"), (int32)ItemId);
        return;
	}
    
	EItemId NewWeaponId = EItemId::NONE;
 
    if (WeaponConf->WeaponType == EWeaponTypes::Throwable) {
		for (EItemId Grenade : ThrowablesArray)
		{   
            if (Grenade == ItemId) {
                NewWeaponId = ItemId;
                break;
            }
		}
    }
    else if (WeaponConf->WeaponType == EWeaponTypes::Spike) {
        if (bHasSpike) {
            NewWeaponId = EItemId::SPIKE;
        }
	}
    else {
        if (RifleState.ItemId == ItemId) {
            NewWeaponId = RifleState.ItemId;
        }
        else if (PistolState.ItemId == ItemId) {
            NewWeaponId = PistolState.ItemId; // corrected to use ItemId directly
        }
        else if (MeleeState.ItemId == ItemId) {
            NewWeaponId = MeleeState.ItemId;
		}
    }

    if (NewWeaponId == EItemId::NONE) {
        UE_LOG(LogTemp, Warning, TEXT("HandleEquipWeapon: Failed to spawn weapon actor for %d"), (int32)ItemId);
        return;
    }
	CurrentWeaponId = NewWeaponId;

	// update speed based on weapon
    if (Character) {
        Character->HandleUpdateSpeedWalkCurrently();
	}

    if (bIsAiming) {
		bIsAiming = false;
    }
}



void UWeaponComponent::OnNewItemPickup(int32 NewInventoryId) {
    bool ShouldEquipNow = true;

    /*if (ShouldEquipNow) {
		EquipWeapon(NewInventoryId);
    }*/
}

EWeaponTypes UWeaponComponent::GetCurrentWeaponType() {
    if (UGameManager::Get(GetWorld())) {
        UWeaponData* WeaponConf = UGameManager::Get(GetWorld())->GetWeaponDataById(CurrentWeaponId);
        if (WeaponConf) {
            return WeaponConf->WeaponType;
        }
    }
	return EWeaponTypes::Unarmed;
}

EWeaponSubTypes UWeaponComponent::GetCurrentWeaponSubType() {
    if (UGameManager::Get(GetWorld())) {
        UWeaponData* WeaponConf = UGameManager::Get(GetWorld())->GetWeaponDataById(CurrentWeaponId);
        if (WeaponConf) {
            return WeaponConf->WeaponSubType;
        }
    }
    return EWeaponSubTypes::None;
}

void UWeaponComponent::DropWeapon() {
	UE_LOG(LogTemp, Warning, TEXT("DropWeapon called"));
    if (CanDropWeapon(CurrentWeaponId)) {
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
	if (CurrentWeaponId == EItemId::NONE) {
        UE_LOG(LogTemp, Warning, TEXT("HandleDropWeapon: No weapon to drop"));
        return;
	}
	UWeaponData* WeaponConf = UGameManager::Get(GetWorld())->GetWeaponDataById(CurrentWeaponId);

    if (!WeaponConf) {
        UE_LOG(LogTemp, Warning, TEXT("HandleDropWeapon: No weapon data found for %d"), (int32)CurrentWeaponId);
        return;
    }

    if (!CanDropWeapon(CurrentWeaponId)) {
        UE_LOG(LogTemp, Warning, TEXT("HandleDropWeapon: Current weapon can not be dropped"));
        return;
    }

	// spawn new pickup item on map
	FVector DropPoint = GetOwner()->GetActorLocation() + FVector(0.f, 0.f, 60.f) + Character->GetActorForwardVector() * 30;
    FPickupData Data;
	Data.Location = DropPoint;
	Data.Amount = 1;
	Data.ItemId = CurrentWeaponId;
	Data.Id = UGameManager::Get(GetWorld())->GetNextItemOnMapId();

    FVector LookDir = Character->GetControlRotation().Vector();
    FVector LaunchVelocity = LookDir * 600.f;

    // Spawn Pickup item
    APickupItem* Pickup = UGameManager::Get(GetWorld())->CreatePickupActor(Data);
    if (Data.ItemId == EItemId::SPIKE) {
        ASpikeMode* SpikeGM = Cast<ASpikeMode>(UGameplayStatics::GetGameMode(GetWorld()));
		bHasSpike = false;
        if (SpikeGM) {
            SpikeGM->NotifyPlayerSpikeState(Character, false);
        }
    }

    if (Pickup && Pickup->GetItemMesh())
    {
        Pickup->PlayerDropInfo(Character);
		UE_LOG(LogTemp, Warning, TEXT("HandleDropWeapon: Dropped weapon %d at location %s"), (int32)CurrentWeaponId, *DropPoint.ToString());
        Pickup->GetItemMesh()->AddImpulse(LaunchVelocity, NAME_None, true);
    }

	if (WeaponConf->WeaponType == EWeaponTypes::Throwable) {
		// remove from throwables array
		ThrowablesArray.Remove(CurrentWeaponId);
	}
    else {
        if (RifleState.ItemId == CurrentWeaponId) { // corrected to use ItemId
            RifleState.ItemId = EItemId::NONE; // update the RifleState
        }
        else if (PistolState.ItemId == CurrentWeaponId) { // corrected to use ItemId
            PistolState.ItemId = EItemId::NONE; // update the PistolState
        }
    }

    // refresh overlapping actors
    RefreshOverlapPickupActors();

	AutoEquipBestWeapon();
}

void UWeaponComponent::RefreshOverlapPickupActors() {
    UCapsuleComponent* Cap = Character->GetCapsuleComponent();
    TArray<AActor*> OverlappingActors;
    Cap->GetOverlappingActors(OverlappingActors);
    for (AActor* A : OverlappingActors)
    {
        APickupItem* Item = Cast<APickupItem>(A);
        if (Item && !Item->IsJustDropped(Character))
        {
            UE_LOG(LogTemp, Warning, TEXT("Overlapping PickupItem: %s"), *Item->GetName());
			// call pickup component to manually trigger overlap
			UPickupComponent* PickupComp = Character->GetPickupComponent();
            if (PickupComp) {
                PickupComp->PickupItem(Item);
            }
        }
    }
}

void UWeaponComponent::StartReload() {
    if (!Character->IsAlive()) return;

    if (!bIsReloading) {
        if (CurrentWeaponId == EItemId::NONE) {
            UE_LOG(LogTemp, Warning, TEXT("StartReload: No weapon equipped"));
            return;
		}

		FWeaponState* WeaponState = GetWeaponStateByItemId(CurrentWeaponId);
        if (!WeaponState) {
            UE_LOG(LogTemp, Warning, TEXT("StartReload: No weapon state found for %d"), (int32)CurrentWeaponId);
			return;
		}

		// if full clip, no need to reload
		UWeaponData* WeaponConf = UGameManager::Get(GetWorld())->GetWeaponDataById(CurrentWeaponId);
        if (!WeaponConf) {
            UE_LOG(LogTemp, Warning, TEXT("StartReload: No weapon data found for %d"), (int32)CurrentWeaponId);
			return;
		}

        if (WeaponState->AmmoInClip >= WeaponConf->MaxAmmoInClip) {
            UE_LOG(LogTemp, Warning, TEXT("StartReload: Clip is already full for %d"), (int32)CurrentWeaponId);
			return;
		}

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
    if (!Character) {
        return false;
    }
    if (!Character->IsAlive())
    {
        return false;
    }

    if (Character->IsRunning()) {
		return false;
    }
    if (bIsReloading) {
        return false;
    }
    if (bIsPlantingSpike || bIsDefusingSpike) {
        return false;
	}
    if (CurrentWeaponId == EItemId::NONE) {
        return false;
    }
  
    // check has ammo left
    if (RifleState.ItemId == CurrentWeaponId) {
        if (RifleState.AmmoInClip <= 0) {
            return false;
        }
    }
    else if (PistolState.ItemId == CurrentWeaponId) {
        if (PistolState.AmmoInClip <= 0) {
            return false;
        }
	}
    
    return true;
}

void UWeaponComponent::OnInput_StartAttack() {
    if (!Character || !Character->IsAlive()) return;

    if (CurrentWeaponId == EItemId::NONE) {
        UE_LOG(LogTemp, Warning, TEXT("HandleStartFire: No weapon equipped"));
        return;
	}
	UE_LOG(LogTemp, Warning, TEXT("OnLeftClickStart called"));

    // is fire arm
	UWeaponData* WeaponConf = UGameManager::Get(GetWorld())->GetWeaponDataById(CurrentWeaponId);

    if (!WeaponConf) {
        UE_LOG(LogTemp, Warning, TEXT("OnLeftClickStart: No weapon data found for %d"), (int32)CurrentWeaponId);
        return;
	}

    if (WeaponConf->WeaponType == EWeaponTypes::Firearm) {
        if (CanShoot()) {
            bIsFiring = true;
            float timeBetweenShots = 0.2f; // Example value, adjust as needed

            OnFire();
            GetOwner()->GetWorldTimerManager().SetTimer(FireTimerHandle, this, &UWeaponComponent::OnFire, timeBetweenShots, true);
        }

		FWeaponState* WeaponState = GetWeaponStateByItemId(CurrentWeaponId);

        if (WeaponState && WeaponState->AmmoInClip <= 0) {
            if (AWeaponFirearm* Firearm = Cast<AWeaponFirearm>(CurrentWeapon)) {
                Firearm->PlayOutOfAmmoSound();
            }
        }
    }
    else if (WeaponConf->WeaponType == EWeaponTypes::Melee) {
         ServerDoMeleeAttack(0);
    }
    else if (WeaponConf->WeaponType == EWeaponTypes::Throwable) {
        if (!bIsPriming) {
            //DrawProjectileCurve();
            ServerSetIsPriming(true);
        }
	}
}

void UWeaponComponent::OnInput_StopAttack() {
    if (bIsFiring) {
        GetOwner()->GetWorldTimerManager().ClearTimer(FireTimerHandle);
        bIsFiring = false;
	}

    UWeaponData* WeaponConf = UGameManager::Get(GetWorld())->GetWeaponDataById(CurrentWeaponId);
    if (!WeaponConf) {
        return;
    }
    if (WeaponConf->WeaponType == EWeaponTypes::Throwable) {
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
	ThrowablesArray.Remove(CurrentWeaponId);
	bIsThrowing = true;
    // Logic throw, move object, and explode
    // Because on server, there's no current weapon, so we need to handle this
	// gen object throwable
    FVector StartPos = Character->GetThrowableLocation();
    //StartPos += Character->GetActorForwardVector() * 10.f; // avoid collision                       // raise a bit if needed
    

    AThrownProjectile* ThrownProj = nullptr;
    
	UWeaponData* WeaponConf = UGameManager::Get(GetWorld())->GetWeaponDataById(CurrentWeaponId);
    if (WeaponConf->WeaponSubType == EWeaponSubTypes::Smoke) {
        ThrownProj = GetWorld()->SpawnActor<AThrownProjectileSmoke>(
            AThrownProjectileSmoke::StaticClass(),
            StartPos,
            FRotator::ZeroRotator);
    }
    else if (WeaponConf->WeaponSubType == EWeaponSubTypes::Stun) {
        ThrownProj = GetWorld()->SpawnActor<AThrownProjectileStun>(
            AThrownProjectileStun::StaticClass(),
            StartPos,
            FRotator::ZeroRotator);
	}
    else if (WeaponConf->WeaponSubType == EWeaponSubTypes::Incendiary) {
        ThrownProj = GetWorld()->SpawnActor<AThrownProjectileIncendiary>(
            AThrownProjectileIncendiary::StaticClass(),
            StartPos,
            FRotator::ZeroRotator);
	}
    else {
        ThrownProj = GetWorld()->SpawnActor<AThrownProjectile>(
            AThrownProjectileFrag::StaticClass(),
            StartPos,
            FRotator::ZeroRotator);
    }
    if (ThrownProj) {
		ThrownProj->SetOwner(GetOwner());
		ThrownProj->SetInstigator(Cast<APawn>(GetOwner()));
        ThrownProj->InitFromData(WeaponConf);
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

	// destroy current weapon and also remove it from array
	ThrowablesArray.Remove(CurrentWeaponId);

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
		FName HitBoneName = TEXT("");
        
		// Trace to find hit point and bone name
		FVector Start = CameraLocation;
		FVector End = Start + ShotDirection * 100000.f;
		FHitResult Hit;
		FCollisionQueryParams Params;
		Params.AddIgnoredActor(Character);

        bool bHit = GetWorld()->LineTraceSingleByChannel(
            Hit, Start, End, ECC_Visibility, Params
        );

		if (bHit) {
            HitBoneName = Hit.BoneName;
		}
        else {
            HitBoneName = TEXT("None");
		}

        FVector TargetPoint = bHit ? Hit.ImpactPoint : End;
		ServerOnFire(CameraLocation, TargetPoint, HitBoneName);
		// effect fire, no need to wait server
		PlayEffectFire(TargetPoint);
    }
}

void UWeaponComponent::ServerOnFire_Implementation(const FVector& StartPoint, const FVector& TargetPoint, FName HitBoneName) {
	HandleOnFire(StartPoint, TargetPoint, HitBoneName);
}

void UWeaponComponent::ServerDoMeleeAttack_Implementation(int AttackIdx) {
    if (!Character->IsAlive()) return;

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
	MulticastDoMeleeAttack(AttackIdx);
}

void UWeaponComponent::MulticastDoMeleeAttack_Implementation(int AttackIdx) {
    Character->PlayMeleeAttackAnimation(AttackIdx);
}


// Server function
void UWeaponComponent::HandleOnFire(const FVector& StartPos, const FVector& TargetPoint, FName HitBoneName) {
    if (GetOwner()->HasAuthority()) // only server makes changes
    {
        if (!Character) {
            return;
        }

        // check current weapon
        if (CurrentWeaponId == EItemId::NONE) {
			UE_LOG(LogTemp, Warning, TEXT("OnFire: Server no current weapon"));
            return;
        }
		if (!CanShoot()) {
            UE_LOG(LogTemp, Warning, TEXT("OnFire: Server can not shoot now"));
            return;
		}
        UE_LOG(LogTemp, Warning, TEXT("OnFire: Server hit bone: %s"), *HitBoneName.ToString());

		// decrease ammo
		bool NeedUpdateAmmo = false;
        if (CurrentWeaponId == RifleState.ItemId) {
			RifleState.AmmoInClip = FMath::Max(0, RifleState.AmmoInClip - 1);
            if (RifleState.AmmoInClip == 0) {
                NeedUpdateAmmo = true;
            }
        }
        else if (CurrentWeaponId == PistolState.ItemId) {
            PistolState.AmmoInClip = FMath::Max(0, PistolState.AmmoInClip - 1);
            if (PistolState.AmmoInClip == 0) {
                NeedUpdateAmmo = true;
			}
        }
        if (NeedUpdateAmmo) {
            HandleReload();
		}

		// validate, prevent cheating
        FVector ServerEye = Character->GetPawnViewLocation();

        // Distance check (client cannot send start behind a wall or very far)
        float Dist = FVector::Dist(ServerEye, StartPos);

        if (Dist > 60.f)   // about half a meter
        {
            UE_LOG(LogTemp, Warning, TEXT("OnFire: Client StartPos is invalid! (%f)"), Dist);
            return;
        }

        FVector ServerForward = Character->GetBaseAimRotation().Vector().GetSafeNormal();
		FVector ClientDirNorm = (TargetPoint - StartPos).GetSafeNormal();

        float Dot = FVector::DotProduct(ServerForward, ClientDirNorm);

        if (Dot < 0.7f)   // limit 45 deviation
        {
            UE_LOG(LogTemp, Warning, TEXT("OnFire: Client aim direction invalid"));
            return;
        }


        // start from head or eyes
        FVector Start = StartPos;
        FVector End = StartPos + ClientDirNorm * 100000.f;

        FHitResult Hit;
        FCollisionQueryParams Params;
        Params.AddIgnoredActor(Character);

        bool bHit = false;
        if (HitBoneName != "None")
        {
            // use precise trace if we have bone name
            bHit = GetWorld()->LineTraceSingleByChannel(
                Hit, Start, End, ECC_Pawn, Params
            );
        }

		UWeaponData* WeaponConf = UGameManager::Get(GetWorld())->GetWeaponDataById(CurrentWeaponId);
        if (!WeaponConf) {
            UE_LOG(LogTemp, Warning, TEXT("OnFire: Server no weapon data for %d"), (int32)CurrentWeaponId);
            return;
		}
        float Damage = WeaponConf->Damage;

        //DrawDebugLine(
        //    GetWorld(),
        //    Start,
        //    TargetPoint,
        //    FColor::Red,
        //    false,      // persistent lines?
        //    3.0f,       // life time
        //    0,          // depth priority
        //    1.5f        // thickness
        //);
        
        if (bHit) {
            AActor* HitActor = Hit.GetActor();
            if (HitActor)
            {
                FMyPointDamageEvent DamageEvent;
                DamageEvent.DamageTypeClass = UMyDamageType::StaticClass();
                DamageEvent.WeaponID = CurrentWeaponId;
                float Multiplier = 1.f;
                UE_LOG(LogTemp, Warning, TEXT("Hit Component: %s"), *Hit.GetComponent()->GetName());

                const FString Bone = HitBoneName.ToString();

                // === Head ===
                if (HitBoneName == TEXT("head"))
                {
                    Multiplier = 5.0f;
                    DamageEvent.bIsHeadshot = true;
                }
                // === Upper body ===
                else if (Bone.StartsWith(TEXT("neck")) ||
                    Bone.Contains(TEXT("spine")) ||
                    Bone.Contains(TEXT("clavicle")) ||
                    HitBoneName == TEXT("pelvis"))
                {
                    Multiplier = 1.25f;
                }
                // === Arms ===
                else if (Bone.Contains(TEXT("arm")) ||
                    Bone.Contains(TEXT("hand")))
                {
                    Multiplier = 0.75f;
                }
                // === Legs ===
                else if (Bone.Contains(TEXT("thigh")) ||
                    Bone.Contains(TEXT("calf")) ||
                    Bone.Contains(TEXT("foot")))
                {
                    Multiplier = 0.75f;
                }

                float FinalDamage = Damage * Multiplier;

                float ActualDamage = HitActor->TakeDamage(FinalDamage, DamageEvent, Character->GetController(), nullptr);
                UE_LOG(LogTemp, Warning, TEXT("OnFire: Server applied damage: %f"), ActualDamage);
            }
        }

        UE_LOG(LogTemp, Warning, TEXT("OnFire: Server calling MulticastPlayFireRifle"));
        MulticastPlayFireRifle(TargetPoint);
    }
}

void UWeaponComponent::PlayEffectFire(FVector TargetPoint) {
    // print log
	UE_LOG(LogTemp, Warning, TEXT("PlayEffectFire called"));

    if (!CurrentWeapon) {
        return;
    }
	UWeaponData* WeaponConf = CurrentWeapon->GetWeaponData();
    
    if (WeaponConf->WeaponType == EWeaponTypes::Firearm) {
        ABaseCharacter* Player = Cast<ABaseCharacter>(GetOwner());

        if (WeaponConf->WeaponSubType == EWeaponSubTypes::Rifle) {
            Player->PlayFireRifleMontage(TargetPoint);
        }
        else if (WeaponConf->WeaponSubType == EWeaponSubTypes::Pistol) {
            Player->PlayFirePistolMontage(TargetPoint);
		}

        if (IsLocalControl()) {
            // Get view point
            FVector CameraLocation;
            FRotator CameraRotation;
            Character->Controller->GetPlayerViewPoint(CameraLocation, CameraRotation);

            CurrentWeapon->OnFire(TargetPoint, true, CameraLocation);
		}
        else {
            CurrentWeapon->OnFire(TargetPoint);
        }
    }
}

void UWeaponComponent::MulticastPlayFireRifle_Implementation(FVector TargetPoint) {
    if (IsNetMode(NM_DedicatedServer)) return;

	if (IsLocalControl()) {
        return; // skip local player
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
        if (WeaponData && WeaponData->WeaponType == EWeaponTypes::Firearm && WeaponData->HasScopeEquiped)
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
	DOREPLIFETIME(UWeaponComponent, CurrentWeaponId);
	DOREPLIFETIME(UWeaponComponent, RifleState);
	DOREPLIFETIME(UWeaponComponent, PistolState);
	DOREPLIFETIME(UWeaponComponent, MeleeState);
	DOREPLIFETIME(UWeaponComponent, ThrowablesArray);
    //DOREPLIFETIME(UWeaponComponent, ThrowablesArray);
}

// Clients press 1, 2, 3 to equip weapon in that slot
void UWeaponComponent::EquipSlot(int32 SlotIndex)
{
	if (!Character->IsAlive()) return;
    if (bIsPlantingSpike || bIsDefusingSpike) {
        return; // can not change weapon while planting or defusing spike
    }

	UE_LOG(LogTemp, Warning, TEXT("EquipSlot called for slot %d"), SlotIndex);
    if (bIsPriming) {
        return; // can not change weapon while priming
    }
  
    if (SlotIndex == FGameConstants::SLOT_THROWABLE) {
		UE_LOG(LogTemp, Warning, TEXT("EquipSlot: Equipping throwable"));
		EItemId Id = EItemId::NONE;

		if (ThrowablesArray.Num() == 0) {
            UE_LOG(LogTemp, Warning, TEXT("EquipSlot: No throwables available"));
			return; // no throwables available
		}

        if (CurrentWeaponId == EItemId::NONE) {
            UE_LOG(LogTemp, Warning, TEXT("EquipSlot: No current weapon, equipping first throwable"));
            Id = ThrowablesArray[0];
        }
        else {
            UWeaponData* WeaponConf = UGameManager::Get(GetWorld())->GetWeaponDataById(CurrentWeaponId);
            if (WeaponConf->WeaponType != EWeaponTypes::Throwable)
            {
                Id = ThrowablesArray[0];
            }
            else if (WeaponConf->WeaponType == EWeaponTypes::Throwable) {
                // Cycle to next available throwable
                int32 CurrentIndex = ThrowablesArray.IndexOfByKey(CurrentWeaponId);
                if (CurrentIndex == INDEX_NONE) {
                    Id = ThrowablesArray[0];
                } else {
                    int32 NextIndex = (CurrentIndex + 1) % ThrowablesArray.Num();
                    Id = ThrowablesArray[NextIndex];
                }
            }
        }
        if (Id == EItemId::NONE) {
            UE_LOG(LogTemp, Warning, TEXT("EquipSlot: No valid throwable Id found"));
			return;
		}

		ServerEquipWeapon(Id);
    }
    else if (SlotIndex == FGameConstants::SLOT_RIFLE) {
        if (RifleState.ItemId != EItemId::NONE) {
			UE_LOG(LogTemp, Warning, TEXT("EquipSlot: Equipping rifle"));
            ServerEquipWeapon(RifleState.ItemId);
		}
    }
    else if (SlotIndex == FGameConstants::SLOT_MELEE) {
        if (MeleeState.ItemId != EItemId::NONE) {
            ServerEquipWeapon(MeleeState.ItemId);
        }
    }
    else if (SlotIndex == FGameConstants::SLOT_PISTOL) {
        if (PistolState.ItemId != EItemId::NONE) {
            ServerEquipWeapon(PistolState.ItemId);
        }
    }
    else if (SlotIndex == FGameConstants::SLOT_SPIKE) {
		UE_LOG(LogTemp, Warning, TEXT("EquipSlot: Equipping spike"));
        if (bHasSpike) {
            ServerEquipWeapon(EItemId::SPIKE);
		}
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

void UWeaponComponent::UpdateAttachLocationWeapon() {
    if (!CurrentWeapon || !Character) {
        return;
    }

    FVector offset = FVector(0.f, 0.f, 0.f);
    FVector offsetRot = FVector(0.f, 0.f, 0.f);
    FString SocketName = "ik_hand_gun";
    bool bIsFPS = Character->IsFpsViewMode();
    UWeaponData* WeaponConf = CurrentWeapon->GetWeaponData();
    if (!WeaponConf) {
        UE_LOG(LogTemp, Warning, TEXT("UpdateAttachLocationWeapon: No weapon data found"));
        return;
    }
    EWeaponTypes WeaType = CurrentWeapon->GetWeaponType();
    if (WeaType == EWeaponTypes::Firearm) {
        if (bIsFPS) {
            SocketName = "ik_hand_gun";
            if (WeaponConf->WeaponSubType == EWeaponSubTypes::Pistol) {
                SocketName = "ik_fps_pistol";
            }
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
    else if (WeaType == EWeaponTypes::Spike) {
        SocketName = "throwable_socket";
    }

    CurrentWeapon->AttachToComponent(Character->GetCurrentMesh(),
        FAttachmentTransformRules::SnapToTargetNotIncludingScale,
        FName(SocketName)
    );
    USceneComponent* Root = CurrentWeapon->GetRootComponent();

    Root->SetRelativeLocationAndRotation(offset, FRotator::MakeFromEuler(offsetRot));

    UE_LOG(LogTemp, Warning, TEXT("UpdateAttachLocationWeapon: Update"));
    if (Character->ViewmodelCapture) {
        CurrentWeapon->SetViewFps(bIsFPS);

        Character->ViewmodelCapture->ShowOnlyComponents.AddUnique(CurrentWeapon->GetWeaponMesh());

        if (CurrentWeapon->GetWeaponType() == EWeaponTypes::Firearm) {
            if (AWeaponFirearm* Firearm = Cast<AWeaponFirearm>(CurrentWeapon))
            {
                if (Firearm->MagMesh) {
                    Character->ViewmodelCapture->ShowOnlyComponents.AddUnique(Firearm->MagMesh);
                    Firearm->MagMesh->SetOwnerNoSee(bIsFPS);
                }
            }
            Character->SetPosViewmodelCaptureForGun();
        }
        else {
            Character->ViewmodelCapture->SetRelativeLocationAndRotation(
                FVector3d::ZeroVector,
                FRotator::ZeroRotator
            );
        }
    }
}

bool UWeaponComponent::CanWeaponAim() {
    if (CurrentWeapon == nullptr) {
        return false;
    }
    if (CurrentWeapon->GetWeaponType() != EWeaponTypes::Firearm) {
        return false;
    }
    if (!IsScopeEquipped()) {
        return false;
	}
    return true;
}

void UWeaponComponent::PerformMeleeAttack(int AttackIdx)
{
	UE_LOG(LogTemp, Warning, TEXT("PerformMeleeAttack called"));
    if (!Character) return;
    //Character->LookInput;
    FVector Start = Character->GetActorLocation() + FVector(0, 0, 70);
    FRotator CamRot = Character->GetControlRotation();


    FVector LookDir = CamRot.Vector();
    FVector End = Start + LookDir * 150;
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
			
            FMyPointDamageEvent DamageEvent;
            DamageEvent.DamageTypeClass = UMyDamageType::StaticClass();
            DamageEvent.WeaponID = CurrentWeaponId;

			UWeaponData* WeaponConf = UGameManager::Get(GetWorld())->GetWeaponDataById(CurrentWeaponId);

            float ActualDamage = Hit.GetActor()->TakeDamage(WeaponConf->Damage, DamageEvent, Character->GetController(), nullptr);
        }
    }
}


void UWeaponComponent::OnRep_CurrentWeapon()
{
    if (!UGameManager::Get(GetWorld())) {
		UE_LOG(LogTemp, Warning, TEXT("OnRep_CurrentWeapon: UGameManager::Get(GetWorld()) is null"));
        return;
    }
    if (CurrentWeaponId == EItemId::NONE) {
        if (CurrentWeapon) {
            CurrentWeapon->Destroy();
            CurrentWeapon = nullptr;
        }
        return;
    }

	UE_LOG(LogTemp, Warning, TEXT("OnRep_CurrentWeapon called for weapon id %d"), (int32)CurrentWeaponId);

	UWeaponData* WeaponConf = UGameManager::Get(GetWorld())->GetWeaponDataById(CurrentWeaponId);
    if (!WeaponConf) {
        UE_LOG(LogTemp, Warning, TEXT("OnRep_CurrentWeapon: No weapon data found for id %d"), (int32)CurrentWeaponId);
		return;
	}

	// destroy old weapon
    if (CurrentWeapon) {
        CurrentWeapon->Destroy();
		CurrentWeapon = nullptr;
	}
   
    CurrentWeapon = SpawnWeaponByItemId(CurrentWeaponId);
    if (!CurrentWeapon) {
        UE_LOG(LogTemp, Warning, TEXT("OnRep_CurrentWeapon: Failed to spawn weapon for id %d"), (int32)CurrentWeaponId);
        return;
    }
   
	CurrentWeapon->SetOwner(GetOwner());
	CurrentWeapon->SetInstigator(Cast<APawn>(GetOwner()));
	CurrentWeapon->InitFromData(WeaponConf);
    UpdateAttachLocationWeapon();

	// Play equip animation
    if (Character) {
        EWeaponTypes WeaType = CurrentWeapon->GetWeaponType();
        Character->PlayEquipWeaponAnimation(WeaType);

        // update speed
        Character->HandleUpdateSpeedWalkCurrently();
	}

    OnUpdateCurrentWeapon.Broadcast(CurrentWeaponId);
}

void UWeaponComponent::ServerReload_Implementation()
{
	HandleReload();
}

void UWeaponComponent::HandleReload()
{
    if (!Character->IsAlive()) return;
    UE_LOG(LogTemp, Warning, TEXT("OnEquipWeaponFinished called"));
    if (bIsReloading) {
        return; // already reloading
    }
    // check can reload
	UWeaponData* WeaponConf = UGameManager::Get(GetWorld())->GetWeaponDataById(CurrentWeaponId);
    if (!WeaponConf) {
		return;
	}

    if (WeaponConf->WeaponType != EWeaponTypes::Firearm) {
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
    if (CurrentWeapon) {
        if (Character) {
            Character->PlayReloadMontage(CurrentWeapon->GetWeaponData());
        }

        if (AWeaponFirearm* Firearm = Cast<AWeaponFirearm>(CurrentWeapon))
        {
            Firearm->PlayReloadSound();
		}
	}
}

void UWeaponComponent::OnFinishedReload()
{
    if (!Character->IsAlive()) return;
    if (GetOwner()->HasAuthority()) // only server makes changes
    {
        bIsReloading = false;
		UE_LOG(LogTemp, Warning, TEXT("OnFinishedReload called"));
		UWeaponData* WeaponConf = UGameManager::Get(GetWorld())->GetWeaponDataById(CurrentWeaponId);
		FWeaponState* WeaponState = GetWeaponStateByItemId(CurrentWeaponId);

		if (!WeaponConf || !WeaponState) {
			return;
		}

		int AmmoNeeded = WeaponConf->MaxAmmoInClip - WeaponState->AmmoInClip;
		if (AmmoNeeded <= 0) {
			return; // clip is full
		}

		int AmmoToReload = FMath::Min(AmmoNeeded, WeaponState->AmmoReserve);
		WeaponState->AmmoReserve = FMath::Max(0, WeaponState->AmmoReserve - AmmoToReload);
		WeaponState->AmmoInClip += AmmoToReload;
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

void UWeaponComponent::OnNewItemAdded(int32 NewInventoryId)
{
    
}

AWeaponBase* UWeaponComponent::SpawnWeaponByItemId(EItemId ItemId)
{
    UWeaponData* WeaponConf = Cast<UWeaponData>(UGameManager::Get(GetWorld())->GetItemDataById(ItemId));
    if (!WeaponConf) {
        UE_LOG(LogTemp, Warning, TEXT("SpawnWeaponByItemId: No weapon data found for id %d"), (int32)ItemId);
        return nullptr;
    }
    FActorSpawnParameters Params;
    Params.Owner = GetOwner();
    Params.Instigator = Cast<APawn>(GetOwner());
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    AWeaponBase* NewWeapon = nullptr;
    if (WeaponConf->WeaponType == EWeaponTypes::Firearm) {
        NewWeapon = GetWorld()->SpawnActor<AWeaponFirearm>(
            FVector::ZeroVector,
            FRotator::ZeroRotator,
            Params
        );
    }
    else if (WeaponConf->WeaponType == EWeaponTypes::Melee) {
        NewWeapon = GetWorld()->SpawnActor<AWeaponMelee>(
            FVector::ZeroVector,
            FRotator::ZeroRotator,
            Params
        );
    }
    else if (WeaponConf->WeaponType == EWeaponTypes::Throwable) {
        NewWeapon = GetWorld()->SpawnActor<AWeaponThrowable>(
            FVector::ZeroVector,
            FRotator::ZeroRotator,
            Params
        );
    }
    else if (WeaponConf->WeaponType == EWeaponTypes::Spike) {
        NewWeapon = GetWorld()->SpawnActor<AWeaponBase>(
            FVector::ZeroVector,
            FRotator::ZeroRotator,
            Params
        );
	}
    if (NewWeapon) {
        NewWeapon->InitFromData(WeaponConf);
    }
    return NewWeapon;
}

// Logic server only
bool UWeaponComponent::AddNewWeapon(FPickupData PickupData)
{
	EItemId ItemId = PickupData.ItemId;
	UWeaponData* WeaponConf = UGameManager::Get(GetWorld())->GetWeaponDataById(ItemId);
    if (!WeaponConf) {
		return false;
	}

    if (WeaponConf->WeaponType == EWeaponTypes::Firearm) {
        if (WeaponConf->WeaponSubType == EWeaponSubTypes::Rifle) {
            if (RifleState.ItemId != EItemId::NONE) {
                return false; // already have rifle
            }
			RifleState.ItemId = ItemId;
            int TotalAmmo = WeaponConf->AmmoBonusShop;
			RifleState.AmmoInClip = PickupData.AmmoInClip;
			RifleState.AmmoReserve = PickupData.AmmoReserve;
			EquipWeapon(ItemId);
            return true;
        }
        else if (WeaponConf->WeaponSubType == EWeaponSubTypes::Pistol) {
			if (PistolState.ItemId != EItemId::NONE) {
				return false; // already have pistol
			}
			PistolState.ItemId = ItemId;
			int TotalAmmo = WeaponConf->AmmoBonusShop;
			PistolState.AmmoInClip = PickupData.AmmoInClip;
			PistolState.AmmoReserve = PickupData.AmmoReserve;
			EquipWeapon(ItemId);
            return true;
        }
        else {
            return false; // unknown firearm subtype
        }
    }
    else if (WeaponConf->WeaponType == EWeaponTypes::Throwable) {
        UE_LOG(LogTemp, Warning, TEXT("AddNewWeapon: Throwable"));
		ThrowablesArray.Add(ItemId);
        ThrowablesArray.Sort([](const EItemId& A, const EItemId& B) {
            return static_cast<int32>(A) < static_cast<int32>(B);
            });
    }
    else if (WeaponConf->WeaponType == EWeaponTypes::Spike) {
		// check if player is allowed to have spike
        AMyPlayerState* MyPS = Cast<AMyPlayerState>(Character->GetPlayerState());
		AShooterGameState* GS = Cast<AShooterGameState>(GetWorld()->GetGameState());
        if (GS->GetAttackerTeam() != MyPS->GetTeamID()) {
            return false; // only attackers can have spike
        }
        
		bHasSpike = true;

		// speical case, need to tell game mode that player has spike
        ASpikeMode* SpikeGM = Cast<ASpikeMode>(UGameplayStatics::GetGameMode(GetWorld()));
        if (SpikeGM) {
            SpikeGM->NotifyPlayerSpikeState(Character, true);
        }
    }
    return true;
}

bool UWeaponComponent::CanDropWeapon(EItemId Id)
{
    if (!Character || !Character->IsAlive()) return false;

    UWeaponData* WeaponConf = UGameManager::Get(GetWorld())->GetWeaponDataById(Id);
    if (!WeaponConf) {
        return false;
    }
	return WeaponConf->CanDrop;
}

FWeaponState* UWeaponComponent::GetWeaponStateByItemId(EItemId ItemId)
{
    if (RifleState.ItemId == ItemId) {
        return &RifleState;
    }
    else if (PistolState.ItemId == ItemId) {
        return &PistolState;
    }
    else if (MeleeState.ItemId == ItemId) {
        return &MeleeState;
    }
	return nullptr;
}

void UWeaponComponent::OnRep_RifleState() {
    if (CurrentWeaponId == RifleState.ItemId) {
        UpdateStateCurrentWeapon();
	}
	OnUpdateRifleWeapon.Broadcast(RifleState.ItemId);
}

void UWeaponComponent::OnRep_PistolState() {
    if (CurrentWeaponId == PistolState.ItemId) {
        UpdateStateCurrentWeapon();
    }
	OnUpdatePistolWeapon.Broadcast(PistolState.ItemId);
}

void UWeaponComponent::OnRep_MeleeState() {

}

void UWeaponComponent::UpdateStateCurrentWeapon()
{
	FWeaponState* WeaponState = GetWeaponStateByItemId(CurrentWeaponId);
    if (WeaponState) {
		OnUpdateAmmoState.Broadcast(WeaponState->AmmoInClip, WeaponState->AmmoReserve);
    }
}

void UWeaponComponent::OnRep_Grenades() {
    OnUpdateGrenades.Broadcast(ThrowablesArray);
}

void UWeaponComponent::TriggerUpdateUI() {
	OnRep_CurrentWeapon();
	OnRep_Grenades();
	OnRep_RifleState();
	OnRep_PistolState();
}

int UWeaponComponent::GetCurrentAmmoInClip() {
    FWeaponState* WeaponState = GetWeaponStateByItemId(CurrentWeaponId);
    if (WeaponState) {
        return WeaponState->AmmoInClip;
    }
    return 0;
}

bool UWeaponComponent::CanReload() {
    FWeaponState* WeaponState = GetWeaponStateByItemId(CurrentWeaponId);
    if (WeaponState) {
        return WeaponState->AmmoReserve > 0;
    }
    return false;
}

void UWeaponComponent::ServerStartPlantSpike_Implementation() {
    if (bHasSpike == false) {
        return; // no spike to plant
    }
   
    if (bIsPlantingSpike) {
        return;
	}
    if (CurrentWeaponId != EItemId::SPIKE) {
		return; // not equipping spike
	}

    if (!CanPlantSpikeAtCurrentLocation()) {
        UE_LOG(LogTemp, Warning, TEXT("ServerStartPlantSpike: Cannot plant spike at current location"));
        return;
    }
	UE_LOG(LogTemp, Warning, TEXT("ServerStartPlantSpike called"));

	bIsPlantingSpike = true;
    Character->ServerSetCrouching(true);

    GetWorld()->GetTimerManager().SetTimer(
        SpikePlantTimerHandle,
        this,
        &UWeaponComponent::FinishPlantSpike,
        3.0f,     // delay
        false     // non-looping
    );
}

void UWeaponComponent::ServerStopPlantSpike_Implementation() {
	UE_LOG(LogTemp, Warning, TEXT("ServerStopPlantSpike called"));
    if (bHasSpike == false) {
        return; // no spike to plant
    }

    if (!bIsPlantingSpike) {
		return;
    }
	bIsPlantingSpike = false;
    GetWorld()->GetTimerManager().ClearTimer(SpikePlantTimerHandle);
}

void UWeaponComponent::MulticastStartPlantSpike_Implementation() {
  /*  if (Character) {
        Character->PlayPlantSpikeMontage();
    }
    bHasSpike = false;*/
}

void UWeaponComponent::MulticastStopPlantSpike_Implementation() {
    /*  if (Character) {
          Character->PlayPlantSpikeMontage();
      }
      bHasSpike = false;*/
}


void UWeaponComponent::ServerStartDefuseSpike_Implementation() {
	UE_LOG(LogTemp, Warning, TEXT("ServerStartDefuseSpike called"));
    // Validate
    // check spike is planted in game mode
	ASpikeMode* SpikeGM = Cast<ASpikeMode>(UGameplayStatics::GetGameMode(GetWorld()));
    if (!SpikeGM) {
        UE_LOG(LogTemp, Warning, TEXT("ServerStartDefuseSpike: No SpikeGM found"));
        return;
	}
	AShooterGameState* GameState = Cast<AShooterGameState>(GetWorld()->GetGameState());

    ASpike* SpikeActor = SpikeGM->GetPlantedSpike();
    if (GameState->GetMatchState() != EMyMatchState::SPIKE_PLANTED) {
		return; // can only defuse during playing state
    }
    if (!SpikeActor) {
        UE_LOG(LogTemp, Warning, TEXT("ServerStartDefuseSpike: No planted spike found"));
        return;
	}

    if (SpikeActor->IsDefuseInProgress()) {
		UE_LOG(LogTemp, Warning, TEXT("ServerStartDefuseSpike: Defuse already in progress"));
        return;
	}


    if (SpikeActor->IsDefused()) {
		UE_LOG(LogTemp, Warning, TEXT("ServerStartDefuseSpike: Spike is already defused"));
        return;
	}

    // check team
	AMyPlayerState* MyPS = Cast<AMyPlayerState>(Character->GetPlayerState());
    if (MyPS->GetTeamID() == GameState->GetAttackerTeam()) {
        UE_LOG(LogTemp, Warning, TEXT("ServerStartDefuseSpike: Attackers cannot defuse spike"));
        return; // attackers cannot defuse
	}

	bIsDefusingSpike = true;
    SpikeActor->StartDefuse(this);
    Character->ServerSetCrouching(true);
}

void UWeaponComponent::FinishDefuseSpike() {
	bIsDefusingSpike = false;
    // check spike is planted in game mode
    UE_LOG(LogTemp, Warning, TEXT("FinishDefuseSpike called"));
    Character->ServerSetCrouching(false);
}

void UWeaponComponent::ServerStopDefuseSpike_Implementation() {
    if (!bIsDefusingSpike) {
        return;
    }
	bIsDefusingSpike = false;
    Character->ServerSetCrouching(false);

	ASpikeMode* SpikeGM = Cast<ASpikeMode>(UGameplayStatics::GetGameMode(GetWorld()));
    ASpike* SpikeActor = SpikeGM->GetPlantedSpike();
    if (SpikeActor) {
        if (SpikeActor->IsDefuseInProgress()) {
            SpikeActor->CancelDefuse();
        }
	}
}

void UWeaponComponent::OnInput_StartPlantSpike() {
	UE_LOG(LogTemp, Warning, TEXT("OnInput_StartPlantSpike called"));
    if (bHasSpike == false) {
        return; // no spike to plant
    }
    if (bIsPlantingSpike) {
        return;
    }
    if (!CanPlantSpikeAtCurrentLocation()) {
        UE_LOG(LogTemp, Warning, TEXT("OnInput_StartPlantSpike: Cannot plant spike at current location"));
        return;
	}
	ServerStartPlantSpike();
}

bool UWeaponComponent::CanPlantSpikeAtCurrentLocation() {
    if (!Character) {
        return false;
    }
    
    if (AActorManager::Get(GetWorld()) == nullptr) {
        UE_LOG(LogTemp, Warning, TEXT("CanPlantSpikeAtCurrentLocation: ActorManager instance is null"));
        return false;
	}
    TArray<ATriggerBox*> BombAreas = {
        AActorManager::Get(GetWorld())->GetAreaBombA(),
        AActorManager::Get(GetWorld())->GetAreaBombB()
    };
    for (ATriggerBox* Area : BombAreas)
    {
        if (!Area)
        {
            UE_LOG(LogTemp, Warning, TEXT("CanPlantSpikeAtCurrentLocation: Bomb area is invalid"));
            continue;   // skip invalid area
        }

        UBoxComponent* Box = Cast<UBoxComponent>(Area->GetCollisionComponent());
        if (!Box) return false;

        FVector BoxCenter = Box->GetComponentLocation();
        FVector BoxSize = Box->GetScaledBoxExtent();

        FVector CharLoc = Character->GetActorLocation();

        bool bInside = UKismetMathLibrary::IsPointInBox(
            CharLoc,
            BoxCenter,
            BoxSize
        );

        if (bInside)
        {
            UE_LOG(LogTemp, Warning, TEXT("Character IS inside %s"), *Area->GetName());
            return true;
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Character is NOT inside %s"), *Area->GetName());
        }
    }

    return false;
}

void UWeaponComponent::OnInput_StopPlantSpike() {
    if (bHasSpike == false) {
        return; // no spike to plant
    }
    if (!bIsPlantingSpike) {
        return;
	}
    ServerStopPlantSpike();
}

void UWeaponComponent::OnRep_IsPlantingSpike() {
    OnUpdatePlantSpikeState.Broadcast(bIsPlantingSpike);

    // play sound
    if (bIsPlantingSpike) {
        if (Character) {
            Character->PlayPlantSpikeEffect();
        }
    }
    else {
        if (Character) {
            Character->StopPlantSpikeEffect();
        }
	}
}

void UWeaponComponent::FinishPlantSpike() {
    if (bHasSpike == false) {
        return;
    }
    if (!bIsPlantingSpike) {
        return;
    }

    // get spike game mode
	ASpikeMode* SpikeGM = Cast<ASpikeMode>(UGameplayStatics::GetGameMode(GetWorld()));

    if (!SpikeGM) {
        UE_LOG(LogTemp, Warning, TEXT("FinishPlantSpike: No SpikeGM found"));
        return;
    }
    FVector SpikeLocation = Character->GetActorLocation()
        + Character->GetActorForwardVector() * 50.f;
	SpikeGM->PlantSpike(SpikeLocation, Character->GetController());
    bIsPlantingSpike = false;
	bHasSpike = false;

	UE_LOG(LogTemp, Warning, TEXT("FinishPlantSpike called"));
    
    Character->ServerSetCrouching(false);
	// change player equipment
    AutoEquipBestWeapon();
}

void UWeaponComponent::MulticastSpikePlanted_Implementation() {
    
}

void UWeaponComponent::OnInput_StartDefuseSpike() {
	ServerStartDefuseSpike();
}

void UWeaponComponent::OnInput_StopDefuseSpike() {
    ServerStopDefuseSpike();
}

void UWeaponComponent::OnRep_IsDefusingSpike() {
    OnUpdateDefuseSpikeState.Broadcast(bIsDefusingSpike);
    if (bIsDefusingSpike) {
        if (Character) {
            Character->PlayDefuseSpikeEffect();
        }
    }
    else {
        if (Character) {
            Character->StopDefuseSpikeEffect();
        }
	}
}

void UWeaponComponent::OnRep_HasSpike() {
    if (!Character) {
        return;
    }
    if (bHasSpike) {
        AMyPlayerController* PC = Cast<AMyPlayerController>(Character->GetController());

        if (PC && PC->PlayerUI) {
            PC->PlayerUI->ShowNotiToast(FText::FromString(TEXT("You picked up Photon")));
        }
    }
}

bool UWeaponComponent::IsHasSpike() {
    return bHasSpike;
}

void UWeaponComponent::AutoEquipBestWeapon()
{
    if (RifleState.ItemId != EItemId::NONE) {
        EquipWeapon(RifleState.ItemId);
    }
    else if (PistolState.ItemId != EItemId::NONE) {
        EquipWeapon(PistolState.ItemId);
    }
    else if (MeleeState.ItemId != EItemId::NONE) {
        EquipWeapon(MeleeState.ItemId);
    }
}

void UWeaponComponent::OnOwnerDeath() {
    if (bIsPlantingSpike) {
        ServerStopPlantSpike();
	}
    if (bIsDefusingSpike) {
        ServerStopDefuseSpike();
	}

	CurrentWeaponId = EItemId::NONE;
    // drop all weapons
    FVector DropPoint = GetOwner()->GetActorLocation() + Character->GetActorForwardVector() * 30;
    if (bHasSpike) {
        FPickupData Data;
        Data.Location = DropPoint;
        Data.Amount = 1;
        Data.ItemId = EItemId::SPIKE;
        Data.Id = UGameManager::Get(GetWorld())->GetNextItemOnMapId();

        // Spawn Pickup item
        APickupItem* Pickup = UGameManager::Get(GetWorld())->CreatePickupActor(Data);
       
        ASpikeMode* SpikeGM = Cast<ASpikeMode>(UGameplayStatics::GetGameMode(GetWorld()));
        if (SpikeGM) {
            SpikeGM->NotifyPlayerSpikeState(Character, false);
        }
        bHasSpike = false;
    }

    if (RifleState.ItemId != EItemId::NONE) {
        FPickupData Data;
        Data.Location = DropPoint;
        Data.AmmoInClip = RifleState.AmmoInClip;
		Data.AmmoReserve = RifleState.AmmoReserve;
        Data.ItemId = RifleState.ItemId;
        Data.Id = UGameManager::Get(GetWorld())->GetNextItemOnMapId();
        // Spawn Pickup item
        APickupItem* Pickup = UGameManager::Get(GetWorld())->CreatePickupActor(Data);
	}

    if (PistolState.ItemId != EItemId::NONE) {
        FPickupData Data;
        Data.Location = DropPoint;
		Data.AmmoInClip = PistolState.AmmoInClip;
		Data.AmmoReserve = PistolState.AmmoReserve;
        Data.ItemId = PistolState.ItemId;
        Data.Id = UGameManager::Get(GetWorld())->GetNextItemOnMapId();
        // Spawn Pickup item
        APickupItem* Pickup = UGameManager::Get(GetWorld())->CreatePickupActor(Data);
	}
}