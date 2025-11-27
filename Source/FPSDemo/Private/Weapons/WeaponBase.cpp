#include "Weapons/WeaponBase.h"
#include "Components/SphereComponent.h" 
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Characters/BaseCharacter.h"
#include "Projectiles/BulletBase.h"
#include "Weapons/WeaponData.h"

// Sets default values
AWeaponBase::AWeaponBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	WeaponStaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponStaticMesh"));
	WeaponStaticMesh->SetHiddenInGame(true);
	WeaponStaticMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponStaticMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	RootComponent = WeaponMesh;

	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	bReplicates = true;
	SetReplicateMovement(false);
}

void AWeaponBase::PreInitializeComponents()
{
	Super::PreInitializeComponents();
}

// Fix the Cast usage in BeginPlay
void AWeaponBase::BeginPlay()
{
	Super::BeginPlay();
	ACharacter* Character = Cast<ACharacter>(GetOwner());
	if (Character && Character->IsLocallyControlled()) {
		WeaponMesh->SetCastShadow(false);
	}
}

// Called every frame
void AWeaponBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

EWeaponTypes AWeaponBase::GetWeaponType()
{
	if (Data) {
		return Data->WeaponType;
	}
	return EWeaponTypes::Unarmed;
}


// This function will be called on server
void AWeaponBase::InitFromData(UWeaponData* InData)
{
	Data = InData;
	ApplyWeaponData();
}

void AWeaponBase::OnFire(FVector TargetPoint)
{
	// Implement firing logic here
	UE_LOG(LogTemp, Warning, TEXT("WeaponBase OnFire called"));
}

void AWeaponBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AWeaponBase, Data);
}

void AWeaponBase::OnRep_WeaponData()
{
	UE_LOG(LogTemp, Warning, TEXT("WeaponBase OnRep_WeaponData called"));
	ApplyWeaponData();
}

void AWeaponBase::ApplyWeaponData()
{
	if (Data && Data->Mesh)
	{
		UE_LOG(LogTemp, Warning, TEXT("WeaponBase ApplyWeaponData: Setting Mesh"));
		WeaponMesh->SetSkeletalMesh(Data->Mesh);

		RootComponent = WeaponMesh;
		WeaponMesh->SetHiddenInGame(false);
		WeaponStaticMesh->SetHiddenInGame(true);
	}
	else if (Data && Data->StaticMesh)
	{
		UE_LOG(LogTemp, Warning, TEXT("WeaponBase ApplyWeaponData: Setting Static Mesh"));
		WeaponStaticMesh->SetStaticMesh(Data->StaticMesh);
		WeaponStaticMesh->SetHiddenInGame(false);
		WeaponMesh->SetHiddenInGame(true);

		RootComponent = WeaponStaticMesh;
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("WeaponBase ApplyWeaponData: Invalid Data or Mesh"));
	}
	RootComponent->SetWorldScale3D(FVector(1.0f));
}

UMeshComponent* AWeaponBase::GetWeaponMesh()
{
	if (WeaponMesh && !WeaponMesh->bHiddenInGame) {
		return WeaponMesh;
	}
	else if (WeaponStaticMesh && !WeaponStaticMesh->bHiddenInGame) {
		return WeaponStaticMesh;
	}
	return nullptr;
}

void AWeaponBase::SetOwnerNoSee(bool bNewOwnerNoSee)
{
	if (WeaponMesh)
	{
		WeaponMesh->SetOwnerNoSee(bNewOwnerNoSee);
	}
	if (WeaponStaticMesh)
	{
		WeaponStaticMesh->SetOwnerNoSee(bNewOwnerNoSee);
	}
}

void AWeaponBase::OnUnequipped()
{
	// Default implementation does nothing
	this->SetActorHiddenInGame(true);
	this->SetActorEnableCollision(false);	
	this->SetActorTickEnabled(false);
}

void AWeaponBase::OnEquipped()
{
	// Default implementation does nothing
	this->SetActorHiddenInGame(false);
	this->SetActorEnableCollision(true);
	this->SetActorTickEnabled(true);
}