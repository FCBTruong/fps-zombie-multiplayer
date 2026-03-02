#include "Game/Items/EquippedItem.h"
#include "Shared/Data/Items/ItemConfig.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"

AEquippedItem::AEquippedItem()
{
    // If you don't actually need Tick, disable it for perf.
    PrimaryActorTick.bCanEverTick = false;

    // Stable root (do not swap RootComponent at runtime)
    Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
    SetRootComponent(Root);

    WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
    WeaponMesh->SetupAttachment(Root);

    WeaponStaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
    WeaponStaticMesh->SetupAttachment(Root);

    auto DisableCollision = [](UMeshComponent* M)
        {
            M->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            M->SetCollisionResponseToAllChannels(ECR_Ignore);
            M->SetGenerateOverlapEvents(false);
        };

    DisableCollision(WeaponMesh);
    DisableCollision(WeaponStaticMesh);

    WeaponMesh->SetHiddenInGame(true);
    WeaponStaticMesh->SetHiddenInGame(true);
    ActiveMesh = EActiveMesh::None;
}

void AEquippedItem::InitFromConfig(const UItemConfig* InData)
{
    if (!InData) return;
    if (Config == InData) return; // avoid re-applying if same config

    Config = InData;
    ApplyConfig();
}

UMeshComponent* AEquippedItem::GetMainMesh() const
{
    switch (ActiveMesh)
    {
    case EActiveMesh::Skeletal: return WeaponMesh;
    case EActiveMesh::Static:  return WeaponStaticMesh;
    default:                   return nullptr;
    }
}

void AEquippedItem::SetActiveMeshSkeletal(USkeletalMesh* InMesh)
{
    // Avoid redundant work
    if (ActiveMesh == EActiveMesh::Skeletal && WeaponMesh->GetSkeletalMeshAsset() == InMesh)
        return;

    WeaponMesh->SetSkeletalMesh(InMesh);
    WeaponMesh->SetHiddenInGame(false, true);

    WeaponStaticMesh->SetHiddenInGame(true, true);
    WeaponStaticMesh->SetStaticMesh(nullptr); // optional: free reference

    ActiveMesh = EActiveMesh::Skeletal;
}

void AEquippedItem::SetActiveMeshStatic(UStaticMesh* InMesh)
{
    if (ActiveMesh == EActiveMesh::Static && WeaponStaticMesh->GetStaticMesh() == InMesh)
        return;

    WeaponStaticMesh->SetStaticMesh(InMesh);
    WeaponStaticMesh->SetHiddenInGame(false, true);

    WeaponMesh->SetHiddenInGame(true, true);
    WeaponMesh->SetSkeletalMesh(nullptr); // optional: free reference

    ActiveMesh = EActiveMesh::Static;
}

void AEquippedItem::ApplyConfig()
{
    if (!Config)
    {
        return;
    }

    if (Config->Mesh)
    {
        SetActiveMeshSkeletal(Config->Mesh);
    }
    else if (Config->StaticMesh)
    {
        SetActiveMeshStatic(Config->StaticMesh);
    }
    else
    {
        WeaponMesh->SetHiddenInGame(true, true);
        WeaponStaticMesh->SetHiddenInGame(true, true);
        ActiveMesh = EActiveMesh::None;
    }
}

void AEquippedItem::SetViewFps(bool bIsFps)
{
    bIsFpsView = bIsFps;
    if (!Config)
    {
        return;
    }

    // Apply scale to stable root (one place, consistent)
    if (bIsFps) {
        WeaponMesh->SetFirstPersonPrimitiveType(EFirstPersonPrimitiveType::FirstPerson);
        WeaponStaticMesh->SetFirstPersonPrimitiveType(EFirstPersonPrimitiveType::FirstPerson);
        Root->SetWorldScale3D(FVector(Config->ScaleOnHand));
    }
    else {
        WeaponMesh->SetFirstPersonPrimitiveType(EFirstPersonPrimitiveType::None);
        WeaponStaticMesh->SetFirstPersonPrimitiveType(EFirstPersonPrimitiveType::None);
        Root->SetWorldScale3D(FVector(Config->ScaleOnHandTps));
    }
}
