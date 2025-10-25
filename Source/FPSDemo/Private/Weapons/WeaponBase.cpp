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

	RootComponent = WeaponMesh;

	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
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


void AWeaponBase::InitFromData(UWeaponData* InData)
{
	Data = InData;
	if (Data && Data->Mesh)
	{
		UE_LOG(LogTemp, Warning, TEXT("WeaponBase InitFromData: Setting Mesh"));
		WeaponMesh->SetSkeletalMesh(Data->Mesh);
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("WeaponBase InitFromData: Invalid Data or Mesh"));
	}
}

void AWeaponBase::OnFire(FVector TargetPoint)
{
	// Implement firing logic here
	UE_LOG(LogTemp, Warning, TEXT("WeaponBase OnFire called"));
}