// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/Items/Pickup/PickupItem.h"
#include "Shared/System/ItemsManager.h"
#include "Game/Characters/BaseCharacter.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Game/Characters/Components/PickupComponent.h"
#include "Net/UnrealNetwork.h"
#include "Shared/Data/Items/ItemConfig.h"
#include "Game/Subsystems/ActorManager.h"

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

void APickupItem::SetData(const FPickupData& NewData)
{
	Data = NewData;
	OnLoadData();
}

void APickupItem::OnLoadData(){
	auto ItemsMgr = UItemsManager::Get(GetWorld());
    // local variable
    const UItemConfig* WeaponData = ItemsMgr->GetItemById(Data.ItemId);
	if (!WeaponData)
	{
		UE_LOG(LogTemp, Warning, TEXT("OnLoadData: No WeaponData found for ItemId %d"), static_cast<int32>(Data.ItemId));
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("OnLoadData: Retrieved WeaponData for ItemId %d"), static_cast<int32>(Data.ItemId));
    if (WeaponData->StaticMesh && ItemMesh)
    {
        ItemMesh->SetStaticMesh(WeaponData->StaticMesh);
    }

	// refactor later, set spike reference
	if (Data.ItemId == EItemId::SPIKE)
	{
		AActorManager* ActorMgr = AActorManager::Get(GetWorld());
		if (ActorMgr)
		{
			ActorMgr->SetPickupSpike(this);
		}
	}
}

void APickupItem::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	if (!bIsActive)
	{
		return;
	}
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
		UPickupComponent* PC = Player->GetPickupComponent();
		if (PC && PC->IsEnabled()) {
			const UItemConfig* ItemConfig = UItemsManager::Get(GetWorld())->GetItemById(this->Data.ItemId);
			if (ItemConfig->GetItemType() != EItemType::Firearm) {
				UE_LOG(LogTemp, Warning, TEXT("Player overlapping with item pickup"));
				PC->PickupItem(this);
			}
		}
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

void APickupItem::RecordDropInfo(ABaseCharacter* Character)
{
	if (!Character) return;
	// Record the time of drop
	LastDropTimeMs = FPlatformTime::Seconds() * 1000.0;
	LastOwner = Character;
}