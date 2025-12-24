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
#include "Components/PickupComponent.h"
#include "Components/AnimationComponent.h"
#include "Weapons/WeaponActionState.h"
#include "Weapons/WeaponFirearm.h"
#include "Weapons/WeaponMelee.h"
#include "Weapons/WeaponThrowable.h"
#include "Weapons/WeaponBase.h"

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
    if (!CanAct()) {
        UE_LOG(LogTemp, Warning, TEXT("EquipWeapon: Cannot act now"));
        return;
	}

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
    if (!CanAct()) {
        UE_LOG(LogTemp, Warning, TEXT("HandleEquipWeapon: Cannot act now"));
        return;
	}

    ABaseCharacter* Character = GetCharacter();

    if (CurrentWeaponId == ItemId)
    {
        UE_LOG(LogTemp, Warning, TEXT("HandleEquipWeapon: Weapon %d is "), (int32)ItemId);
        // already equipped
        return;
    }
    if (!UGameManager::Get(GetWorld())) {
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
        Character->UpdateMaxWalkSpeed();
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

void UWeaponComponent::RequestDropWeapon() {
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
    ABaseCharacter* Character = GetCharacter();
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
    ABaseCharacter* Character = GetCharacter();
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

void UWeaponComponent::RequestReload() {
    ABaseCharacter* Character = GetCharacter();
    if (!Character || !Character->IsAlive()) return;
    if (!CanReload()) {
        return;
    }


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

void UWeaponComponent::StartAiming() {
    bIsAiming = true;
}

// This function only apply for firearms
EShootState UWeaponComponent::CanShoot() const{
    ABaseCharacter* Character = GetCharacter();
    if (!Character) {
        return EShootState::CannotFire; // update return value to EShootState
    }
    if (!Character->IsAlive())
    {
        return EShootState::CannotFire; // update return value to EShootState
    }


    if (CurrentWeaponId == EItemId::NONE) {
        return EShootState::CannotFire; 
    }

    // check has ammo left
    if (RifleState.ItemId == CurrentWeaponId) {
        if (RifleState.AmmoInClip <= 0) {
            return EShootState::OutOfAmmo; 
        }
    }
    else if (PistolState.ItemId == CurrentWeaponId) {
        if (PistolState.AmmoInClip <= 0) {
            return EShootState::OutOfAmmo;
        }
    }

    return EShootState::OK;
}

void UWeaponComponent::ServerThrow_Implementation(FVector LaunchVelocity) {
    if (CurrentWeaponId == EItemId::NONE) {
        UE_LOG(LogTemp, Warning, TEXT("ServerThrow: No weapon equipped"));
        return;
    }
    if (!ThrowablesArray.Contains(CurrentWeaponId)) {
        UE_LOG(LogTemp, Warning, TEXT("ServerThrow: Current weapon is not in throwables array"));
        return;
    }
    if (ActionState != EWeaponActionState::Idle) {
        UE_LOG(LogTemp, Warning, TEXT("ServerThrow: Cannot throw while in action state %d"), (int32)ActionState);
        return;
    }

    ABaseCharacter* Character = GetCharacter();
    FVector StartPos = Character->GetThrowableLocation();
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
    EquipSlot(FGameConstants::SLOT_MELEE);
}

void UWeaponComponent::MulticastThrowAction_Implementation(FVector LaunchVelocity) {
    UE_LOG(LogTemp, Warning, TEXT("MulticastThrowAction called"));
    if (TrajectoryPreviewRef) {
        TrajectoryPreviewRef->Destroy();
        TrajectoryPreviewRef = nullptr;
    }
    ABaseCharacter* Character = GetCharacter();
    if (!Character) {
        return;
    }
    UAnimationComponent* AnimComp = Character->GetAnimationComponent();
    if (AnimComp) {
        AnimComp->PlayThrowNadeMontage();
    }
    GetOwner()->GetWorldTimerManager().ClearTimer(ThrowProjectileTimer);
}

void UWeaponComponent::ServerDoMeleeAttack_Implementation(int AttackIdx) {
    ABaseCharacter* Character = GetCharacter();
    if (!Character->IsAlive()) return;


    // Check if can attack
    if (!Character) {
        return;
    }
    /* if (Character->IsCloseToWall()) {
         return;
     }*/

    MulticastDoMeleeAttack(AttackIdx);
}

void UWeaponComponent::MulticastDoMeleeAttack_Implementation(int AttackIdx) {
 
}

void UWeaponComponent::PlayEffectFire(FVector TargetPoint) {
    // print log
    UE_LOG(LogTemp, Warning, TEXT("PlayEffectFire called"));

    if (!CurrentWeapon) {
        return;
    }
    UWeaponData* WeaponConf = CurrentWeapon->GetWeaponData();

    if (WeaponConf->WeaponType == EWeaponTypes::Firearm) {
        ABaseCharacter* Character = Cast<ABaseCharacter>(GetOwner());

        if (!Character) {
            return;
        }
        UAnimationComponent* AnimComp = Character->GetAnimationComponent();

        if (WeaponConf->WeaponSubType == EWeaponSubTypes::Rifle) {
            AnimComp->PlayFireRifleMontage(TargetPoint);
        }
        else if (WeaponConf->WeaponSubType == EWeaponSubTypes::Pistol) {
            AnimComp->PlayFirePistolMontage(TargetPoint);
        }

        if (IsOwningClient()) {
            // Get view point
            FVector CameraLocation;
            FRotator CameraRotation;
            Character->Controller->GetPlayerViewPoint(CameraLocation, CameraRotation);

            CurrentWeapon->OnFire(TargetPoint);
        }
        else {
            CurrentWeapon->OnFire(TargetPoint);
        }
    }
}

void UWeaponComponent::MulticastPlayFireRifle_Implementation(FVector TargetPoint) {
    if (IsNetMode(NM_DedicatedServer)) return;

    if (IsOwningClient()) {
        return; // skip local player
    }
    PlayEffectFire(TargetPoint);
}

bool UWeaponComponent::IsOwningClient() const{
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

    DOREPLIFETIME(UWeaponComponent, CurrentWeaponId);
    DOREPLIFETIME(UWeaponComponent, RifleState);
    DOREPLIFETIME(UWeaponComponent, PistolState);
    DOREPLIFETIME(UWeaponComponent, MeleeState);
    DOREPLIFETIME(UWeaponComponent, ThrowablesArray);
    DOREPLIFETIME(UWeaponComponent, ArmorState);
    DOREPLIFETIME(UWeaponComponent, ActionState);
    DOREPLIFETIME(UWeaponComponent, BurstSeed);
    DOREPLIFETIME(UWeaponComponent, FireStartTimeServer);
    //DOREPLIFETIME(UWeaponComponent, ThrowablesArray);
}

// Clients press 1, 2, 3 to equip weapon in that slot
void UWeaponComponent::EquipSlot(int32 SlotIndex)
{
    if (!CanAct()) {
        return;
    }

    if (ActionState != EWeaponActionState::Idle) {
        return; // can not change weapon while in action
    }

    UE_LOG(LogTemp, Warning, TEXT("EquipSlot called for slot %d"), SlotIndex);

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
                }
                else {
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
    ABaseCharacter* Character = GetCharacter();
    if (!Character->GetThrowSpline()) {
        UE_LOG(LogTemp, Warning, TEXT("DrawProjectileCurve: SplineRef is invalid"));
    }
    TrajectoryPreviewRef->SplineRef = Character->GetThrowSpline();
}

void UWeaponComponent::UpdateProjectileCurve()
{
    ABaseCharacter* Character = GetCharacter();
    if (!Character || !Character->GetThrowSpline())
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

    Character->GetThrowSpline()->ClearSplinePoints();

    for (const FPredictProjectilePathPointData& Point : Result.PathData)
    {
        Character->GetThrowSpline()->AddSplinePoint(Point.Location, ESplineCoordinateSpace::World);
    }
}

FVector UWeaponComponent::GetVelocityGrenade() const
{
    ABaseCharacter* Character = GetCharacter();
    FRotator ControlRot = Character->GetControlRotation();

    // Add pitch offset (ThrowAngle is a float variable you define)
    FRotator ThrowRot = FRotator(ControlRot.Pitch + ThrowAngle, ControlRot.Yaw, ControlRot.Roll);

    // Convert to forward vector
    FVector Forward = ThrowRot.Vector();

    // Multiply by initial grenade speed (float variable)
    return Forward * GrenadeInitSpeed;
}

void UWeaponComponent::ServerSetIsPriming_Implementation(bool bNewIsPriming)
{
    // change to state: Preparing to throw
    if (bNewIsPriming) {

    }
    else {
		SetActionState(EWeaponActionState::Idle);
    }
}

void UWeaponComponent::UpdateAttachLocationWeapon(bool bIsFPS) {
    ABaseCharacter* Character = GetCharacter();
    if (!CurrentWeapon || !Character) {
        return;
    }
    UE_LOG(LogTemp, Warning, TEXT("UpdateAttachLocationWeapon: Called"));

    FVector offset = FVector(0.f, 0.f, 0.f);
    FVector offsetRot = FVector(0.f, 0.f, 0.f);
    FString SocketName = "ik_hand_gun";
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
    if (Character->GetViewmodelCapture()) {
        CurrentWeapon->SetViewFps(bIsFPS);

        UE_LOG(LogTemp, Warning, TEXT("UpdateAttachLocationWeapon: Update ViewmodelCapture"));
        Character->GetViewmodelCapture()->ShowOnlyComponents.AddUnique(CurrentWeapon->GetWeaponMesh());

        UE_LOG(LogTemp, Warning, TEXT("UpdateAttachLocationWeapon: Set OwnerNoSee to %s"), bIsFPS ? TEXT("true") : TEXT("false"));
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
    // TODO later
    UE_LOG(LogTemp, Warning, TEXT("PerformMeleeAttack called"));
    ABaseCharacter* Character = GetCharacter();
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

    }

    if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Pawn, Params))
    {
        if (AActor* Target = Hit.GetActor())
        {
            UE_LOG(LogTemp, Warning, TEXT("PerformMeleeAttack: Hit actor %s"), *Target->GetName());


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

    UE_LOG(LogTemp, Warning, TEXT("OnRep_CurrentWeapon: Spawned weapon %s"), *CurrentWeapon->GetName());
    ABaseCharacter* Character = GetCharacter();
    if (!Character) {
        UE_LOG(LogTemp, Warning, TEXT("OnRep_CurrentWeapon: No character found"));
        return;
    }

    CurrentWeapon->SetViewCapture(Character->GetViewmodelCapture());

    CurrentWeapon->SetOwner(GetOwner());
    CurrentWeapon->SetInstigator(Cast<APawn>(GetOwner()));
    CurrentWeapon->InitFromData(WeaponConf);
    UpdateAttachLocationWeapon(Character->IsFpsViewMode());

    // Play equip animation
   
    EWeaponTypes WeaType = CurrentWeapon->GetWeaponType();

    UAnimationComponent* AnimComp = Character->GetAnimationComponent();
    if (AnimComp) {
        //AnimComp->PlayEquip(WeaType);
    }
    Character->UpdateMaxWalkSpeed();

    OnUpdateCurrentWeapon.Broadcast(CurrentWeaponId);
}

void UWeaponComponent::ServerReload_Implementation()
{
    HandleReload();
}

void UWeaponComponent::HandleReload()
{
    if (!CanAct()) {
        return;
	}
    
    if (!CanReload()) {
        return;
    }
    UE_LOG(LogTemp, Warning, TEXT("OnEquipWeaponFinished called"));

    if (ActionState != EWeaponActionState::Idle) {
        return; // can not reload while in action
    }
    // check can reload
    UWeaponData* WeaponConf = UGameManager::Get(GetWorld())->GetWeaponDataById(CurrentWeaponId);
    if (!WeaponConf) {
        return;
    }

    if (WeaponConf->WeaponType != EWeaponTypes::Firearm) {
        return; // only firearm can reload
    }
	SetActionState(EWeaponActionState::Reloading);
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
        ABaseCharacter* Character = GetCharacter();

        if (!Character) {
            return;
        }

        UAnimationComponent* AnimComp = Character->GetAnimationComponent();
        if (AnimComp) {
            //AnimComp->PlayReloadMontage();
        }

       /* if (AWeaponFirearm* Firearm = Cast<AWeaponFirearm>(CurrentWeapon))
        {
            Firearm->PlayReloadSound();
        }*/
    }
}

void UWeaponComponent::OnFinishedReload()
{
    ABaseCharacter* Character = GetCharacter();
    if (!Character || !Character->IsAlive()) return;
    if (!GetOwner()->HasAuthority()) {
        return;
    }
	SetActionState(EWeaponActionState::Idle);

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

void UWeaponComponent::OnNotifyGrabMag() {
    UE_LOG(LogTemp, Warning, TEXT("OnNotifyGrabMag called"));
    if (CurrentWeapon) {
        //if (AWeaponFirearm* Firearm = Cast<AWeaponFirearm>(CurrentWeapon))
        //{
        //    if (Firearm->MagMesh == nullptr) {
        //        UE_LOG(LogTemp, Warning, TEXT("OnNotifyInsertMag: MagMesh is null"));
        //        return;
        //    }
        //    /* Firearm->PlayGrabMagSound();*/
        //    UE_LOG(LogTemp, Warning, TEXT("OnNotifyGrabMag: Attaching mag to hand_r"));
        //    Firearm->MagMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
        //    ABaseCharacter* Character = GetCharacter();
        //    if (Character->IsFpsViewMode()) {
        //        Firearm->MagMesh->AttachToComponent(
        //            Character->GetCurrentMesh(),
        //            FAttachmentTransformRules::KeepWorldTransform,
        //            FName("hand_l")
        //        );
        //    }
        //    else {
        //        Firearm->MagMesh->AttachToComponent(
        //            Character->GetCurrentMesh(),
        //            FAttachmentTransformRules::KeepRelativeTransform,
        //            FName("hand_l")
        //        );

        //        // Snap position only
        //        FVector TargetLoc = Character->GetCurrentMesh()->GetSocketLocation("hand_l");
        //        Firearm->MagMesh->SetWorldLocation(TargetLoc);
        //    }
        //}
    }
}

void UWeaponComponent::OnNotifyInsertMag() {
    UE_LOG(LogTemp, Warning, TEXT("OnNotifyInsertMag called"));
    if (CurrentWeapon) {
        /*if (AWeaponFirearm* Firearm = Cast<AWeaponFirearm>(CurrentWeapon))
        {
            if (Firearm->MagMesh == nullptr) {
                UE_LOG(LogTemp, Warning, TEXT("OnNotifyInsertMag: MagMesh is null"));
                return;
            }
            Firearm->MagMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
            Firearm->AttachMagToDefault();
        }*/
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
       /* NewWeapon = GetWorld()->SpawnActor<AWeaponFirearm>(
            FVector::ZeroVector,
            FRotator::ZeroRotator,
            Params
        );*/
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
    ABaseCharacter* Character = GetCharacter();

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
    else if (WeaponConf->WeaponType == EWeaponTypes::Equipment) {
        // hard code for test
        if (ItemId == EItemId::KEVLAR_VEST) {
            ArmorState.ArmorPoints = 100;
            ArmorState.ArmorEfficiency = 0.4f;
            ArmorState.ArmorRatio = 0.3f;
        }
        else {
            // level 2
            ArmorState.ArmorPoints = 100;
            ArmorState.ArmorEfficiency = 0.3f;
            ArmorState.ArmorRatio = 0.5f;
        }
    }
    return true;
}

bool UWeaponComponent::CanDropWeapon(EItemId Id)
{
    if (!CanAct()) {
        return false;
	}
    
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

bool UWeaponComponent::CanReload()
{
    if (CurrentWeaponId == EItemId::NONE) {
        return false;
	}
    if (ActionState == EWeaponActionState::Reloading) {
        return false; // already reloading
    }
    if (!CanTransition(ActionState, EWeaponActionState::Reloading)) {
        return false; // already reloading
	}

    FWeaponState* WeaponState = GetWeaponStateByItemId(CurrentWeaponId);
    if (!WeaponState) return false;

    UWeaponData* Data = UGameManager::Get(GetWorld())->GetWeaponDataById(CurrentWeaponId);
    if (!Data) return false;

    return WeaponState->AmmoReserve > 0 &&
        WeaponState->AmmoInClip < Data->MaxAmmoInClip;
}


void UWeaponComponent::ServerStartPlantSpike_Implementation() {
    // Refactor this later
    if (bHasSpike == false) {
        return; // no spike to plant
    }


    if (CurrentWeaponId != EItemId::SPIKE) {
        return; // not equipping spike
    }

    if (!CanPlantSpikeAtCurrentLocation()) {
        UE_LOG(LogTemp, Warning, TEXT("ServerStartPlantSpike: Cannot plant spike at current location"));
        return;
    }
    UE_LOG(LogTemp, Warning, TEXT("ServerStartPlantSpike called"));

    ABaseCharacter* Character = GetCharacter();
    Character->RequestCrouch();

    GetWorld()->GetTimerManager().SetTimer(
        SpikePlantTimerHandle,
        this,
        &UWeaponComponent::FinishPlantSpike,
        3.0f,     // delay
        false     // non-looping
    );
}

void UWeaponComponent::ServerStopPlantSpike_Implementation() {
    // Need refactor
    UE_LOG(LogTemp, Warning, TEXT("ServerStopPlantSpike called"));
    if (bHasSpike == false) {
        return; // no spike to plant
    }

    /*if (!bIsPlantingSpike) {
        return;
    }*/
    //bIsPlantingSpike = false;
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
    ABaseCharacter* Character = GetCharacter();
    AMyPlayerState* MyPS = Cast<AMyPlayerState>(Character->GetPlayerState());
    if (MyPS->GetTeamID() == GameState->GetAttackerTeam()) {
        UE_LOG(LogTemp, Warning, TEXT("ServerStartDefuseSpike: Attackers cannot defuse spike"));
        return; // attackers cannot defuse
    }

    //bIsDefusingSpike = true;
    SpikeActor->StartDefuse(this);
    Character->RequestCrouch();
}

void UWeaponComponent::FinishDefuseSpike() {
    ABaseCharacter* Character = GetCharacter();
    //bIsDefusingSpike = false;
    // check spike is planted in game mode
    UE_LOG(LogTemp, Warning, TEXT("FinishDefuseSpike called"));
    Character->RequestUnCrouch();
}

void UWeaponComponent::ServerStopDefuseSpike_Implementation() {
    // Need refactor
  /*  if (!bIsDefusingSpike) {
        return;
    }*/
    ABaseCharacter* Character = GetCharacter();
    //bIsDefusingSpike = false;
    Character->RequestUnCrouch();
    ASpikeMode* SpikeGM = Cast<ASpikeMode>(UGameplayStatics::GetGameMode(GetWorld()));
    if (!SpikeGM) {
        UE_LOG(LogTemp, Warning, TEXT("ServerStopDefuseSpike: No SpikeGM found"));
        return;
	}
    ASpike* SpikeActor = SpikeGM->GetPlantedSpike();
    if (SpikeActor) {
        if (SpikeActor->IsDefuseInProgress()) {
            SpikeActor->CancelDefuse();
        }
    }
}

void UWeaponComponent::OnInput_StartPlantSpike() {
    // Refactor this later
    //UE_LOG(LogTemp, Warning, TEXT("OnInput_StartPlantSpike called"));
 //   if (bHasSpike == false) {
 //       return; // no spike to plant
 //   }
 //   if (bIsPlantingSpike) {
 //       return;
 //   }
 //   if (!CanPlantSpikeAtCurrentLocation()) {
 //       UE_LOG(LogTemp, Warning, TEXT("OnInput_StartPlantSpike: Cannot plant spike at current location"));
 //       return;
    //}
    //ServerStartPlantSpike();
}

bool UWeaponComponent::CanPlantSpikeAtCurrentLocation() {
    ABaseCharacter* Character = GetCharacter();
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
    //   if (bHasSpike == false) {
    //       return; // no spike to plant
    //   }
    //   if (!bIsPlantingSpike) {
    //       return;
       //}
    //   ServerStopPlantSpike();
}

//void UWeaponComponent::OnRep_IsPlantingSpike() {
//    OnUpdatePlantSpikeState.Broadcast(bIsPlantingSpike);
//    ABaseCharacter* Character = GetCharacter();
//
//    // play sound
//    if (bIsPlantingSpike) {
//        if (Character) {
//            Character->OnPlantSpikeStarted();
//        }
//    }
//    else {
//        if (Character) {
//            Character->OnPlantSpikeStopped();
//        }
//	}
//}

void UWeaponComponent::FinishPlantSpike() {
    // Refactor this later
    if (bHasSpike == false) {
        return;
    }
    /*if (!bIsPlantingSpike) {
        return;
    }*/

    // get spike game mode
    ASpikeMode* SpikeGM = Cast<ASpikeMode>(UGameplayStatics::GetGameMode(GetWorld()));

    if (!SpikeGM) {
        UE_LOG(LogTemp, Warning, TEXT("FinishPlantSpike: No SpikeGM found"));
        return;
    }
    ABaseCharacter* Character = GetCharacter();
    FVector SpikeLocation = Character->GetActorLocation()
        + Character->GetActorForwardVector() * 50.f;
    SpikeGM->PlantSpike(SpikeLocation, Character->GetController());
    //bIsPlantingSpike = false;
    bHasSpike = false;

    UE_LOG(LogTemp, Warning, TEXT("FinishPlantSpike called"));

    Character->RequestCrouch();
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

//void UWeaponComponent::OnRep_IsDefusingSpike() {
//    ABaseCharacter* Character = GetCharacter();
//    if (!Character) {
//        return;
//	}
//    OnUpdateDefuseSpikeState.Broadcast(bIsDefusingSpike);
//    if (bIsDefusingSpike) {
//        if (Character) {
//            Character->OnDefuseSpikeStarted();
//        }
//    }
//    else {
//        if (Character) {
//            Character->OnDefuseSpikeStopped();
//        }
//	}
//}

void UWeaponComponent::OnRep_HasSpike() {
    ABaseCharacter* Character = GetCharacter();
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

// Server function called when owner dies
void UWeaponComponent::OnOwnerDeath() {
    // Refactor this later
   /* if (bIsPlantingSpike) {
        ServerStopPlantSpike();
    }
    if (bIsDefusingSpike) {
        ServerStopDefuseSpike();
    }*/

    CurrentWeaponId = EItemId::NONE;
    ABaseCharacter* Character = GetCharacter();

    if (!Character) {
        return;
    }

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

void UWeaponComponent::OnRep_ArmorState() {
    OnUpdateArmor.Broadcast(ArmorState.ArmorPoints);
}

ABaseCharacter* UWeaponComponent::GetCharacter() const {
    return Cast<ABaseCharacter>(GetOwner());
}

bool UWeaponComponent::SetActionState(EWeaponActionState NewState)
{
    if (!GetOwner() || !GetOwner()->HasAuthority())
        return false;

    if (!CanTransition(ActionState, NewState))
        return false;

    const EWeaponActionState OldState = ActionState;
    ActionState = NewState;

    OnActionStateChanged(OldState, NewState);
    return true;
}

bool UWeaponComponent::CanTransition(
    EWeaponActionState From,
    EWeaponActionState To
) const
{
    if (From == EWeaponActionState::Reloading && To == EWeaponActionState::Firing)
        return false;

    if (From == EWeaponActionState::Planting && To != EWeaponActionState::Idle)
        return false;

    if (From == EWeaponActionState::Defusing && To != EWeaponActionState::Idle)
        return false;

    return true;
}

void UWeaponComponent::OnActionStateChanged(
    EWeaponActionState OldState,
    EWeaponActionState NewState
)
{
    // Cancel previous action
    switch (OldState)
    {
    case EWeaponActionState::Firing:
        GetWorld()->GetTimerManager().ClearTimer(FireTimer_Client);
		break;
    case EWeaponActionState::Reloading:
        //GetWorld()->GetTimerManager().ClearTimer(ReloadTimerHandle);
        break;

    case EWeaponActionState::Throwing:
        //GetWorld()->GetTimerManager().ClearTimer(ThrowProjectileTimer);
        break;

    default:
        break;
    }

    // Start new action
    switch (NewState)
    {
    case EWeaponActionState::Reloading:
        //StartReload_Internal();
        break;

    case EWeaponActionState::Firing:
        //StartFire_Internal();
        break;

    default:
        break;
    }
}

void UWeaponComponent::OnRep_ActionState(EWeaponActionState OldState)
{
    OnActionStateChanged(OldState, ActionState);
}

bool UWeaponComponent::HasAmmoInClip()
{
    FWeaponState* State = GetWeaponStateByItemId(CurrentWeaponId);
    return State && State->AmmoInClip > 0;
}

void UWeaponComponent::RequestStartFire()
{
    UE_LOG(LogTemp, Warning, TEXT("RequestStartFire called"));
    /*if (!GetOwner()->HasAuthority())
    {
        if (IsOwningClient()) {
			auto ShootState = CanShoot();
            if (ShootState != EShootState::OK)
            {
                if (ShootState == EShootState::OutOfAmmo) {
                    UE_LOG(LogTemp, Warning, TEXT("RequestStartFire: No ammo to shoot"));
                    if (AWeaponFirearm* Firearm = Cast<AWeaponFirearm>(CurrentWeapon)) {
                        Firearm->PlayOutOfAmmoSound();
                    }
				}
                return;
			}
            float TimeNow = GetServerTimeSeconds();

            if (TimeNow - LastShotTimeServer > BurstResetDelay)
            {
                BurstAccDeg = 0.f;
            }
#if !UE_SERVER
            FireOnce_Predicted();
#endif
            GetWorld()->GetTimerManager().SetTimer(FireTimer_Client, this, &UWeaponComponent::FireOnce_Predicted, FireInterval, true);
        }
    }
    
    if (GetOwner()->HasAuthority())
    {
        StartFire_Authority();
    }
    else
    {
        ServerStartFire();
    }*/
}

void UWeaponComponent::RequestStopFire()
{
    if (!GetOwner()->HasAuthority())
    {
        if (IsOwningClient()) {
            GetWorld()->GetTimerManager().ClearTimer(FireTimer_Client);
        }
    }
    
    if (GetOwner()->HasAuthority())
    {
        StopFire_Authority();
    }
    else
    {
        ServerStopFire();
	}
}

void UWeaponComponent::ServerStartFire_Implementation()
{
    UE_LOG(LogTemp, Warning, TEXT("ServerStartFire called"));
    StartFire_Authority();
}

void UWeaponComponent::ServerStopFire_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("ServerStopFire called"));
	StopFire_Authority();
}

void UWeaponComponent::FireOnce_Authority()
{
	UE_LOG(LogTemp, Warning, TEXT("FireOnce called"));
    // Server authoritative trace + damage
    ABaseCharacter* Character = GetCharacter();
    if (!Character || !GetWorld() || !Character->HasAuthority()) return;

    if (CanShoot() != EShootState::OK)
    {
        ServerStopFire();
        return;
    }

    UE_LOG(LogTemp, Warning, TEXT("Server: FireOnce called"));
    // consume ammo
	FWeaponState* WeaponState = GetWeaponStateByItemId(CurrentWeaponId);
    if (WeaponState) {
        WeaponState->AmmoInClip = FMath::Max(0, WeaponState->AmmoInClip - 1);
	}

    const float NowServerTime = GetServerTimeSeconds();
    UpdateBurstSpreadOnShot(NowServerTime);

    FVector Start, AimDir;
    GetAim(Start, AimDir);
    const FVector ShotDir = ComputeShotDirDeterministic(AimDir, NowServerTime, BurstSeed);

    FHitResult Hit;
    FVector ShotEnd;
    const bool bHit = TraceShot(Character, Start, ShotDir, Hit, ShotEnd);

#if !(UE_BUILD_SHIPPING)
    //DrawDebugLine(GetWorld(), Start, bHit ? Hit.ImpactPoint : End, FColor::Red, false, 0.75f, 0, 1.0f);
#endif

    float Damage = 25.0f;
    if (bHit)
    {
        AActor* HitActor = Hit.GetActor();
        if (HitActor)
        {
            FMyPointDamageEvent DamageEvent;
            DamageEvent.DamageTypeClass = UMyDamageType::StaticClass();
            DamageEvent.WeaponID = CurrentWeaponId;
            float Multiplier = 1.f;
            UE_LOG(LogTemp, Warning, TEXT("Hit Component: %s"), *Hit.GetComponent()->GetName());
            DamageEvent.bIsHeadshot = false;


            float FinalDamage = Damage * Multiplier;
            float ActualDamage = HitActor->TakeDamage(FinalDamage, DamageEvent, Character->GetController(), nullptr);
            UE_LOG(LogTemp, Warning, TEXT("OnFire: Server applied damage: %f"), ActualDamage);
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("OnFire: Server calling MulticastPlayFireRifle"));
    MulticastPlayFireRifle(ShotEnd);
}

void UWeaponComponent::FireOnce_Predicted() {
    UE_LOG(LogTemp, Warning, TEXT("FireOnce_Client called"));
    if (CanShoot() != EShootState::OK)
    {
		GetWorld()->GetTimerManager().ClearTimer(FireTimer_Client);
        return;
    }
    ABaseCharacter* Character = GetCharacter();
    if (!Character || !GetWorld()) return;

    const float NowServerTime = GetServerTimeSeconds();
    UpdateBurstSpreadOnShot(NowServerTime);

    FVector Start, AimDir;
    GetAim(Start, AimDir);
    const FVector ShotDir = ComputeShotDirDeterministic(AimDir, NowServerTime, BurstSeed);

    FHitResult Hit;
    FVector ShotEnd;
    const bool bHit = TraceShot(Character, Start, ShotDir, Hit, ShotEnd);
	PlayEffectFire(ShotEnd);
}

void UWeaponComponent::StartFire_Authority()
{
    UWorld* World = GetWorld();
    if (!World) return;

    if (ActionState == EWeaponActionState::Firing)
    {
        UE_LOG(LogTemp, Warning, TEXT("StartFire_Authority: Already firing"));
        return;
    }

    if (!CanTransition(ActionState, EWeaponActionState::Firing))
    {
        UE_LOG(LogTemp, Warning, TEXT("StartFire_Authority: Cannot transition from %d"), (int32)ActionState);
        return;
    }

    if (CanShoot() != EShootState::OK)
    {
        UE_LOG(LogTemp, Warning, TEXT("StartFire_Authority: Cannot shoot (ammo/state)"));
        return;
    }

    SetActionState(EWeaponActionState::Firing);
	FireStartTimeServer = GetServerTimeSeconds();

    float TimeNow = GetServerTimeSeconds();
    if (TimeNow - LastShotTimeServer > BurstResetDelay)
    {
        BurstAccDeg = 0.f;
    }

	LastShotTimeServer = GetServerTimeSeconds();
    BurstSeed = FMath::Rand();

    // Immediate authoritative shot, then loop with interval delay
    FireOnce_Authority();
    World->GetTimerManager().SetTimer(
        FireTimer,
        this,
        &UWeaponComponent::FireOnce_Authority,
        FireInterval,
        /*bLoop=*/true,
        /*FirstDelay=*/FireInterval
    );
}

void UWeaponComponent::StopFire_Authority()
{
    UWorld* World = GetWorld();
    if (!World) return;

    if (ActionState != EWeaponActionState::Firing) return;

    SetActionState(EWeaponActionState::Idle);
    World->GetTimerManager().ClearTimer(FireTimer);
}

void UWeaponComponent::GetAim(FVector& OutStart, FVector& OutDir) const
{
    const AActor* Owner = GetOwner();
    if (!Owner)
        return;

    OutStart = Owner->GetActorLocation();
    OutDir = Owner->GetActorForwardVector();

    if (const ACharacter* Character = Cast<ACharacter>(Owner))
    {
        FRotator ViewRot;
        Character->GetActorEyesViewPoint(OutStart, ViewRot);
        OutDir = ViewRot.Vector();
    }
}

void UWeaponComponent::OnViewModeChanged(bool bIsFPS)
{
	UE_LOG(LogTemp, Warning, TEXT("OnViewModeChanged called: bIsFPS = %d"), bIsFPS);
	UpdateAttachLocationWeapon(bIsFPS);
}

bool UWeaponComponent::TraceShot(
    const AActor* IgnoredActor,
    const FVector& Start,
    const FVector& Dir,
    FHitResult& OutHit,
    FVector& OutEnd
) const
{
    UWorld* World = GetWorld();
    if (!World) return false;

    OutEnd = Start + Dir * 10000.0f;

    FCollisionQueryParams Params(SCENE_QUERY_STAT(WeaponTrace), /*bTraceComplex=*/true);
    if (IgnoredActor)
    {
        Params.AddIgnoredActor(IgnoredActor);
    }

    const bool bHit = World->LineTraceSingleByChannel(
        OutHit,
        Start,
        OutEnd,
        ECC_Visibility,
        Params
    );

    if (bHit)
    {
        OutEnd = OutHit.ImpactPoint;
    }

    return bHit;
}

float UWeaponComponent::GetServerTimeSeconds() const
{
    // GameState server time is replicated and smoothed on clients.
    const UWorld* World = GetWorld();
    if (!World) return 0.0f;

    const AGameStateBase* GS = World->GetGameState();
    if (!GS) return World->GetTimeSeconds();

    return GS->GetServerWorldTimeSeconds();
}

int32 UWeaponComponent::ComputeShotIndex(float NowServerTime) const
{
    if (FireInterval <= 0.0f) return 0;

    const float Elapsed = FMath::Max(0.0f, NowServerTime - FireStartTimeServer);

    // Small epsilon avoids float edge when Elapsed is exactly N*Interval.
    const float Eps = 0.0001f;

    const int32 Index = FMath::FloorToInt((Elapsed + Eps) / FireInterval);
    return FMath::Max(0, Index);
}

float UWeaponComponent::GetMoveAlphaForSpread() const
{
	return 0.f;
}

float UWeaponComponent::GetMovementSpreadDeg() const
{
    return 0.f;
}

float UWeaponComponent::GetAirSpreadDeg() const
{
	return 0.f;
}

void UWeaponComponent::UpdateBurstSpreadOnShot(float NowServerTime)
{
    
}

float UWeaponComponent::GetTotalSpreadDeg(float NowServerTime) const
{
    return .0f; 
}

FVector UWeaponComponent::ComputeShotDirDeterministic(
    const FVector& AimDir,
    float NowServerTime,
    int32 InBurstSeed
) const
{
    const int32 ShotIndex = ComputeShotIndex(NowServerTime);

    // Seed per shot: stable and deterministic
    const int32 PerShotSeed = InBurstSeed ^ (ShotIndex * 196613);
    FRandomStream Stream(PerShotSeed);

    const float SpreadDeg = GetTotalSpreadDeg(NowServerTime);
    const float SpreadRad = FMath::DegreesToRadians(SpreadDeg);

    return Stream.VRandCone(AimDir, SpreadRad, SpreadRad).GetSafeNormal();
}

bool UWeaponComponent::CanAct() const
{
    ABaseCharacter* Character = GetCharacter();
    if (!Character || !Character->IsAlive()) return false;
    return true;
}