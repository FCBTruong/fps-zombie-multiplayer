// Fill out your copyright notice in the Description page of Project Settings.


#include "PickupItem.h"
#include "WeaponDataManager.h"

// Sets default values
APickupItem::APickupItem()
{
	PrimaryActorTick.bCanEverTick = false;

	ItemMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("ItemMesh"));
	RootComponent = ItemMesh;

	PickupSphere = CreateDefaultSubobject<USphereComponent>(TEXT("PickupSphere"));
	PickupSphere->SetupAttachment(RootComponent);
	PickupSphere->SetSphereRadius(50.f);
	PickupSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	PickupSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	PickupSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
}


// Called when the game starts or when spawned
void APickupItem::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APickupItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void APickupItem::SetData(const FPickupData& NewData)
{
    Data = NewData;

    UGameInstance* GI = GetGameInstance();
    if (!GI)
        return;

    UWeaponDataManager* WeaponDataMgr = GI->GetSubsystem<UWeaponDataManager>();
    if (!WeaponDataMgr)
        return;

    // local variable
    UWeaponData* WeaponData = WeaponDataMgr->GetWeaponById(Data.ItemId);
    if (WeaponData && WeaponData->Mesh && ItemMesh)
    {
        ItemMesh->SetSkeletalMesh(WeaponData->Mesh);
    }
}

