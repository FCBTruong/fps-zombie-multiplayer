// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/Characters/Components/ItemVisualComponent.h"
#include "Game/Characters/Components/EquipComponent.h"
#include "Game/GameManager.h"
#include "Game/Characters/BaseCharacter.h"
#include "Game/Items/Weapons/WeaponFirearm.h"
#include "Game/Items/EquippedItem.h"
#include "Shared/System/ItemsManager.h"
#include "Game/Characters/Components/CharCameraComponent.h"
#include "Shared/Data/Items/FirearmConfig.h"
#include "Game/Characters/Components/AnimationComponent.h"

// Sets default values for this component's properties
UItemVisualComponent::UItemVisualComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(false);
}

void UItemVisualComponent::Init()
{
    Character = Cast<ABaseCharacter>(GetOwner());
    check(Character);

    AnimComp = Character->GetAnimationComponent();
    EquipComp = Character->GetEquipComponent();
    CameraComp = Character->GetCharCameraComponent();

    check(EquipComp);
    check(CameraComp);
    check(AnimComp);

    CameraComp->OnViewModeChanged.AddUObject(
        this,
        &UItemVisualComponent::OnViewModeChanged
    );
    EquipComp->OnActiveItemChanged.AddUObject(
        this,
        &UItemVisualComponent::HandleActiveItemChanged
    );
}

void UItemVisualComponent::BeginPlay()
{
    Super::BeginPlay();
    RefreshVisual(EquipComp->GetActiveItemId());
}

void UItemVisualComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    EquipComp->OnActiveItemChanged.RemoveAll(this);
    CameraComp->OnViewModeChanged.RemoveAll(this);
    DestroyVisual();

    Super::EndPlay(EndPlayReason);
}

void UItemVisualComponent::HandleActiveItemChanged(EItemId NewItemId)
{
    RefreshVisual(NewItemId);
    AnimComp->PlayEquipMontage();
}

void UItemVisualComponent::DestroyVisual()
{
    if (EquippedActor)
    {
        EquippedActor->Destroy();
        EquippedActor = nullptr;
    }
}

void UItemVisualComponent::RefreshVisual(EItemId NewItemId)
{
    if (GetWorld()->GetNetMode() == NM_DedicatedServer) {
        return;
    }

    // Remove old visual
    DestroyVisual();

    if (NewItemId == EItemId::NONE)
    {
        return;
    }

    const UItemConfig* Data = GetItemConfig(NewItemId);
    if (!Data)
    {
        return;
    }

    FActorSpawnParameters Params;
    Params.Owner = Character;
    Params.Instigator = Character;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// check if is firearm
    UClass* SpawnClass = (Data->GetItemType() == EItemType::Firearm)
        ? AWeaponFirearm::StaticClass()
        : AEquippedItem::StaticClass();

    EquippedActor = GetWorld()->SpawnActor<AEquippedItem>(
        SpawnClass,
        FVector::ZeroVector,
        FRotator::ZeroRotator,
        Params
    );

    if (!EquippedActor)
    {
        return;
    }

    EquippedActor->InitFromConfig(Data);
    AttachToHands(Data);
}

void UItemVisualComponent::AttachToHands(const UItemConfig* Data)
{
    if (!EquippedActor || !Data)
    {
        return;
    }

    const bool bIsFPS = Character->IsFpsViewMode();
    USkeletalMeshComponent* Mesh = Character->GetCurrentMesh();
	check(Mesh);

    // You can keep your old socket rules here:
    FName SocketName;
    if (bIsFPS)
    {
        SocketName = Data->SocketNameFps;
	}
    else {
        SocketName = Data->SocketNameTps;
    }

    EquippedActor->AttachToComponent(
        Mesh,
        FAttachmentTransformRules::SnapToTargetNotIncludingScale,
        SocketName
    );

    if (bIsFPS) {
        EquippedActor->SetActorRelativeLocation(Data->OffsetFps);
    }
    else {
        EquippedActor->SetActorRelativeLocation(Data->OffsetTps);
	}
    
    EquippedActor->SetViewFps(bIsFPS);
}

void UItemVisualComponent::OnNotifyGrabMag()
{
    AWeaponFirearm* Firearm = Cast<AWeaponFirearm>(EquippedActor);
    if (!Firearm || !Firearm->MagMesh) {
        return;
    }
    Firearm->MagMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);

    USkeletalMeshComponent* Mesh = Character->GetCurrentMesh();
    check(Mesh);
    if (Character->IsFpsViewMode())
    {
        Firearm->MagMesh->AttachToComponent(Mesh,
            FAttachmentTransformRules::KeepWorldTransform,
            FName("hand_l"));
    }
    else
    {
        Firearm->MagMesh->AttachToComponent(Mesh,
            FAttachmentTransformRules::KeepRelativeTransform,
            FName("hand_l"));

        const FVector TargetLoc = Mesh->GetSocketLocation("hand_l");
        Firearm->MagMesh->SetWorldLocation(TargetLoc);
    }
}

void UItemVisualComponent::OnNotifyInsertMag()
{
    AWeaponFirearm* Firearm = Cast<AWeaponFirearm>(EquippedActor);
    if (!Firearm || !Firearm->MagMesh) {
        return;
    }
    Firearm->MagMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
    Firearm->AttachMagToDefault();
}

const UItemConfig* UItemVisualComponent::GetItemConfig(EItemId ItemId) const
{
	UItemsManager* ItemsManager = UItemsManager::Get(GetWorld());
	return ItemsManager->GetItemById(ItemId);
}

void UItemVisualComponent::PlayFireFX(FVector TargetPoint)
{
    AWeaponFirearm* Firearm = Cast<AWeaponFirearm>(EquippedActor);
    if (!Firearm)
    {
        return;
    }
   
    FRotator ViewRot;
    FVector OutStart;
    Character->GetActorEyesViewPoint(OutStart, ViewRot);
    Firearm->OnFire(TargetPoint, true, OutStart);

    // fire montage
	const UItemConfig* Data = EquippedActor->GetItemConfig();
    // cast to firearm
	const UFirearmConfig* FirearmData = Cast<UFirearmConfig>(Data);
    if (!FirearmData) {
		return;
    }

    if (FirearmData->FirearmType == EFirearmType::Rifle) {
        AnimComp->PlayFireRifleMontage(FirearmData->CharFireMontage);
    }
    else {
        AnimComp->PlayFirePistolMontage();
    }
}

void UItemVisualComponent::PlayMeleeAttack(UAnimMontage* Anim)
{
    AnimComp->PlayMontage(Anim);
}

void UItemVisualComponent::HideItemVisual()
{
    if (EquippedActor)
    {
        EquippedActor->SetActorHiddenInGame(true);
    }
}

void UItemVisualComponent::OnOwnerDead()
{
    DestroyVisual();
}


void UItemVisualComponent::OnViewModeChanged(bool bIsFPS)
{
    const UItemConfig* Data = EquipComp ? GetItemConfig(EquipComp->GetActiveItemId()) : nullptr;
    if (!Data || !EquippedActor)
        return;

    // Re-attach with new socket/offset
    AttachToHands(Data);
}
