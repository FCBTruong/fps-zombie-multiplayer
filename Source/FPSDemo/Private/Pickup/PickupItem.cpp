// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickup/PickupItem.h"
#include "Weapons/WeaponDataManager.h"
#include "Characters/BaseCharacter.h"

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
	PickupSphere->SetHiddenInGame(true);

	PickupSphere->OnComponentBeginOverlap.AddDynamic(this, &APickupItem::OnOverlapBegin);
	UE_LOG(LogTemp, Warning, TEXT("PickupItem BeginPlay called"));
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

void APickupItem::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT("Overlap with weapon pickup"));
	if (ABaseCharacter* Player = Cast<ABaseCharacter>(OtherActor))
	{
		Player->GetPickupComponent()->PickupItem(this);
	}
}