#include "Items/EquippedItem.h"
#include "Items/ItemConfig.h"
#include "Components/SceneComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneCaptureComponent2D.h"

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
            if (!M) return;
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

void AEquippedItem::InitFromConfig(UItemConfig* InData)
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
    if (!WeaponMesh || !WeaponStaticMesh) return;

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
    if (!WeaponMesh || !WeaponStaticMesh) return;

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
        UE_LOG(LogTemp, Warning, TEXT("ApplyConfig: Config is null"));
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
        UE_LOG(LogTemp, Warning, TEXT("ApplyConfig: Config has no mesh"));
        if (WeaponMesh) WeaponMesh->SetHiddenInGame(true, true);
        if (WeaponStaticMesh) WeaponStaticMesh->SetHiddenInGame(true, true);
        ActiveMesh = EActiveMesh::None;
        return;
    }
}

void AEquippedItem::SetViewFps(bool bIsFps)
{
    // log display name of data
    UE_LOG(LogTemp, Warning, TEXT("WeaponBase SetOwnerNoSee called with %s for weapon %s"), bIsFps ? TEXT("true") : TEXT("false"), *GetName());
    if (WeaponMesh)
    {
        WeaponMesh->bVisibleInSceneCaptureOnly = bIsFps;
        WeaponMesh->MarkRenderStateDirty();
    }
    if (WeaponStaticMesh)
    {
        WeaponStaticMesh->bVisibleInSceneCaptureOnly = bIsFps;
        WeaponStaticMesh->MarkRenderStateDirty();
    }

    // Apply scale to stable root (one place, consistent)
    if (RootComponent)
    {
        if (bIsFps)
            RootComponent->SetWorldScale3D(FVector(Config->ScaleOnHand));
        else {
            RootComponent->SetWorldScale3D(FVector(Config->ScaleOnHandTps));
        }
    }
    bIsFpsView = bIsFps;
}

void AEquippedItem::SetViewCapture(USceneCaptureComponent2D* InCapture)
{
    ViewmodelCapture = InCapture;
}