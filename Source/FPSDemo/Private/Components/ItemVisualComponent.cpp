// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/ItemVisualComponent.h"
#include "Components/EquipComponent.h"
#include "Game/GameManager.h"
#include "Characters/BaseCharacter.h"
#include "Weapons/WeaponData.h"
#include "Weapons/WeaponBase.h"
#include "Weapons/WeaponFirearm.h"
#include "Items/EquippedItem.h"
#include "Game/ItemsManager.h"
#include "Components/CharCameraComponent.h"
#include "Items/FirearmConfig.h"
#include "Components/AnimationComponent.h"

// Sets default values for this component's properties
UItemVisualComponent::UItemVisualComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(false);
}

void UItemVisualComponent::Initialize(UEquipComponent* InEquipComp, UCharCameraComponent* InCameraComp, UAnimationComponent* InAnimComp)
{
	if (!InEquipComp || !InCameraComp)
        return;
    EquipComp = InEquipComp;

	AnimComp = InAnimComp;
    
    EquipComp->OnActiveItemChanged.AddUObject(
        this,
        &UItemVisualComponent::HandleActiveItemChanged
    );
	CameraComp = InCameraComp;
    CameraComp->OnViewModeChanged.AddUObject(
        this,
        &UItemVisualComponent::OnViewModeChanged
	);
    // Initial visual
    RefreshVisual(EquipComp->GetActiveItemId());
}
void UItemVisualComponent::BeginPlay()
{
    Super::BeginPlay();

    CachedGM = UGameManager::Get(GetWorld());
}

void UItemVisualComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    DestroyVisual();
    Super::EndPlay(EndPlayReason);
}

bool UItemVisualComponent::ShouldSpawnVisuals() const
{
    const UWorld* World = GetWorld();
    if (!World) return false;
    return World->GetNetMode() != NM_DedicatedServer;
}

ABaseCharacter* UItemVisualComponent::GetCharacter() const
{
    return Cast<ABaseCharacter>(GetOwner());
}

void UItemVisualComponent::HandleActiveItemChanged(EItemId NewItemId)
{
	UE_LOG(LogTemp, Log, TEXT("UItemVisualComponent::HandleActiveItemChanged called with ItemId: %d"), static_cast<uint8>(NewItemId));
    RefreshVisual(NewItemId);

	// play equip animation
    if (AnimComp) {
        // get 
        AnimComp->PlayEquipMontage();
    }
}

void UItemVisualComponent::DestroyVisual()
{
    ABaseCharacter* Character = GetCharacter();
    if (Character)
    {
        RemoveFromViewmodelCapture(Character);
    }

    if (EquippedActor)
    {
        EquippedActor->Destroy();
        EquippedActor = nullptr;
    }
}

void UItemVisualComponent::RefreshVisual(EItemId NewItemId)
{
	UE_LOG(LogTemp, Log, TEXT("UItemVisualComponent::RefreshVisual called with ItemId: %d"), static_cast<uint8>(NewItemId));
    if (!ShouldSpawnVisuals())
        return;

    ABaseCharacter* Character = GetCharacter();
    if (!Character)
        return;

    // Remove old visual
    DestroyVisual();

    if (NewItemId == EItemId::NONE)
    {
		UE_LOG(LogTemp, Log, TEXT("UItemVisualComponent: No item equipped, skipping visual spawn."));
        return;
    }

    const UItemConfig* Data = GetItemConfig(NewItemId);
    if (!Data)
        return;

    FActorSpawnParameters Params;
    Params.Owner = GetOwner();
    Params.Instigator = Cast<APawn>(GetOwner());
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	UE_LOG(LogTemp, Log, TEXT("Spawning visual for item id: %d"), static_cast<uint8>(NewItemId));

	// check if is firearm
    if (Data->GetItemType() == EItemType::Firearm)
    {
        UE_LOG(LogTemp, Log, TEXT("Item is a firearm."));
        EquippedActor = GetWorld()->SpawnActor<AWeaponFirearm>(
            AWeaponFirearm::StaticClass(),
            FVector::ZeroVector,
            FRotator::ZeroRotator,
            Params
		);
    }
    else
    {
        EquippedActor = GetWorld()->SpawnActor<AEquippedItem>(
            AEquippedItem::StaticClass(),
            FVector::ZeroVector,
            FRotator::ZeroRotator,
            Params
        );
	}

    if (!EquippedActor)
        return;

    EquippedActor->SetOwner(GetOwner());
    EquippedActor->SetInstigator(Cast<APawn>(GetOwner()));
    EquippedActor->InitFromConfig(const_cast<UItemConfig*>(Data));

	UE_LOG(LogTemp, Log, TEXT("EquippedActor spawned: %s"), *EquippedActor->GetName());

    // Setup view capture (if you used it before)
   // EquippedActor->SetViewCapture(Character->GetViewmodelCapture());

    AttachToHands(Data);
}

void UItemVisualComponent::AttachToHands(const UItemConfig* Data)
{
    ABaseCharacter* Character = GetCharacter();
    if (!Character || !EquippedActor || !Data)
        return;

    const bool bIsFPS = Character->IsFpsViewMode();
    USkeletalMeshComponent* Mesh = Character->GetCurrentMesh();
    if (!Mesh)
        return;

    // You can keep your old socket rules here:
    FName SocketName = "ik_hand_gun";
    if (bIsFPS)
    {
        SocketName = Data->SocketNameFps;
	}
    else {
        SocketName = Data->SocketNameTps;
    }
    FVector Offset = FVector::ZeroVector;
    FVector OffsetRotEuler = FVector::ZeroVector;

    EquippedActor->AttachToComponent(
        Mesh,
        FAttachmentTransformRules::SnapToTargetNotIncludingScale,
        SocketName
    );

    if (USceneComponent* Root = EquippedActor->GetRootComponent())
    {
       // Root->SetRelativeLocationAndRotation(Offset, FRotator::MakeFromEuler(OffsetRotEuler));
    }

    // FPS capture visibility
    if (Character->GetViewmodelCapture())
    {
        EquippedActor->SetViewFps(bIsFPS);
		EquippedActor->SetViewCapture(Character->GetViewmodelCapture());
        AddToViewmodelCapture(Character);
    }
}

void UItemVisualComponent::OnViewModeChanged(bool bIsFPS)
{
    const UItemConfig* Data = EquipComp ? GetItemConfig(EquipComp->GetActiveItemId()) : nullptr;
    if (!Data || !EquippedActor)
        return;

    // Re-attach with new socket/offset
    AttachToHands(Data);
}

void UItemVisualComponent::RemoveFromViewmodelCapture(ABaseCharacter* Character)
{
    if (!Character || !Character->GetViewmodelCapture() || !EquippedActor)
        return;

    // Remove previous component from ShowOnly list (prevents list growth)
    Character->GetViewmodelCapture()->ShowOnlyComponents.Remove(EquippedActor->GetMainMesh());
}

void UItemVisualComponent::AddToViewmodelCapture(ABaseCharacter* Character)
{
    if (!Character || !Character->GetViewmodelCapture() || !EquippedActor)
        return;

    Character->GetViewmodelCapture()->ShowOnlyComponents.AddUnique(EquippedActor->GetMainMesh());
}

void UItemVisualComponent::OnNotifyGrabMag()
{
   /* if (!EquippedActor) return;

    if (AWeaponFirearm* Firearm = Cast<AWeaponFirearm>(EquippedActor))
    {
        ABaseCharacter* Character = GetCharacter();
        if (!Character) return;

        if (!Firearm->MagMesh) return;

        Firearm->MagMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);

        if (Character->IsFpsViewMode())
        {
            Firearm->MagMesh->AttachToComponent(Character->GetCurrentMesh(),
                FAttachmentTransformRules::KeepWorldTransform,
                FName("hand_l"));
        }
        else
        {
            Firearm->MagMesh->AttachToComponent(Character->GetCurrentMesh(),
                FAttachmentTransformRules::KeepRelativeTransform,
                FName("hand_l"));

            const FVector TargetLoc = Character->GetCurrentMesh()->GetSocketLocation("hand_l");
            Firearm->MagMesh->SetWorldLocation(TargetLoc);
        }
    }*/
}

void UItemVisualComponent::OnNotifyInsertMag()
{
   /* if (!EquippedActor) return;

    if (AWeaponFirearm* Firearm = Cast<AWeaponFirearm>(EquippedActor))
    {
        if (!Firearm->MagMesh) return;
        Firearm->MagMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
        Firearm->AttachMagToDefault();
    }*/
}

const UItemConfig* UItemVisualComponent::GetItemConfig(EItemId ItemId) const
{
	UItemsManager* ItemsManager = UItemsManager::Get(GetWorld());
	return ItemsManager ? ItemsManager->GetItemById(ItemId) : nullptr;
}


void UItemVisualComponent::PlayFireFX(FVector TargetPoint)
{
	UE_LOG(LogTemp, Log, TEXT("UItemVisualComponent::PlayFireFX called with TargetPoint: %s"), *TargetPoint.ToString());
    if (!EquippedActor)
        return;
    if (AWeaponFirearm* Firearm = Cast<AWeaponFirearm>(EquippedActor))
    {
        Firearm->OnFire(TargetPoint, false, FVector::ZeroVector);
    }
    // fire montage
	ABaseCharacter* Character = GetCharacter();
    if (Character && AnimComp)
    {
		const UItemConfig* Data = EquippedActor->GetItemConfig();
        // cast to firearm
		const UFirearmConfig* FirearmData = Cast<UFirearmConfig>(Data);
        if (FirearmData) {
            if (FirearmData->FirearmType == EFirearmType::Rifle) {
				AnimComp->PlayFireRifleMontage(TargetPoint);
                return;
			}
            else {
				AnimComp->PlayFirePistolMontage(TargetPoint);
				return;
            }
        }
	}
}

void UItemVisualComponent::PlayMeleeAttack(int32 AttackIndex)
{
    ABaseCharacter* Character = GetCharacter();
    if (Character && AnimComp)
    {
        AnimComp->PlayMeleeAttackMontage(AttackIndex);
    }
}