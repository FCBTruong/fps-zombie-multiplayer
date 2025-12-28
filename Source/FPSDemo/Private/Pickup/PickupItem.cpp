// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickup/PickupItem.h"
#include "Game/ItemsManager.h"
#include "Characters/BaseCharacter.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Components/PickupComponent.h"
#include "Net/UnrealNetwork.h"
#include "Items/ItemConfig.h"

// Sets default values
APickupItem::APickupItem()
{
	ItemMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemMesh"));
	RootComponent = ItemMesh;

	ItemMesh->SetCollisionProfileName(TEXT("PhysicsActor"));
	ItemMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	ItemMesh->SetSimulatePhysics(true);
	ItemMesh->CanCharacterStepUpOn = ECB_Yes;

	PickupSphere = CreateDefaultSubobject<USphereComponent>(TEXT("PickupSphere"));
	PickupSphere->SetupAttachment(RootComponent);
	PickupSphere->SetSphereRadius(50.f);
	PickupSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	PickupSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	PickupSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	PickupSphere->SetHiddenInGame(true);

	PickupSphere->OnComponentBeginOverlap.AddDynamic(this, &APickupItem::OnOverlapBegin);
	UE_LOG(LogTemp, Warning, TEXT("PickupItem Constructor called"));
    // Replace direct access to bReplicateMovement with the public setter function
    bReplicates = true;
    SetReplicateMovement(true);
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
	OnLoadData();
}

void APickupItem::OnLoadData(){
	auto ItemsMgr = UItemsManager::Get(GetWorld());
    // local variable
    const UItemConfig* WeaponData = ItemsMgr->GetItemById(Data.ItemId);
	UE_LOG(LogTemp, Warning, TEXT("OnLoadData: Retrieved WeaponData for ItemId %d"), static_cast<int32>(Data.ItemId));
    if (WeaponData && WeaponData->StaticMesh && ItemMesh)
    {
        ItemMesh->SetStaticMesh(WeaponData->StaticMesh);
    }
}

void APickupItem::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	// only allow on server
	if (!HasAuthority())
	{
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("Overlap with weapon pickup"));
	if (ABaseCharacter* Player = Cast<ABaseCharacter>(OtherActor))
	{
		if (!Player)
		{
			UE_LOG(LogTemp, Warning, TEXT("OverlapBegin: OtherActor is not ABaseCharacter"));
			return;
		}
		if (IsJustDropped(Player))
		{
			UE_LOG(LogTemp, Warning, TEXT("OverlapBegin: Item was just dropped by this player, ignoring pickup"));
			return;
		}
		Player->GetPickupComponent()->PickupItem(this);
	}
}

bool APickupItem::IsJustDropped(ABaseCharacter* Character) const
{
	if (Character != LastOwner) {
		return false;
	}
	double CurrentTimeMs = FPlatformTime::Seconds() * 1000.0;
	return (CurrentTimeMs - LastDropTimeMs) < 500.0;
}

FString APickupItem::GetItemName() const
{
	FString Name = TEXT("Unknown Item");

	const UItemConfig* WeaponData = UItemsManager::Get(GetWorld())->GetItemById(Data.ItemId);
	if (WeaponData)
	{
		Name = WeaponData->DisplayName.ToString();
	}
	
	return Name;
}

void APickupItem::OnRep_Data()
{
	OnLoadData();
}

void APickupItem::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APickupItem, Data);
}

void APickupItem::PlayerDropInfo(ABaseCharacter* Character)
{
	if (!Character) return;
	// Record the time of drop
	LastDropTimeMs = FPlatformTime::Seconds() * 1000.0;
	LastOwner = Character;
	// Optionally, you can add logic here to prevent immediate re-pickup by the dropping player
}