#include "WeaponBase.h"
#include "Components/SphereComponent.h" 
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "BaseCharacter.h"
#include "BulletBase.h"

// Sets default values
AWeaponBase::AWeaponBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	PrimaryActorTick.bCanEverTick = false;

	PickupSphere = CreateDefaultSubobject<USphereComponent>(TEXT("PickupSphere"));
	if (PickupSphere)
	{
		PickupSphere->InitSphereRadius(100.f);
		PickupSphere->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
		RootComponent = PickupSphere;
	}
}

// Called when the game starts or when spawned
void AWeaponBase::BeginPlay()
{
	Super::BeginPlay();
    // Fix: Change AWeaponPickup to AWeaponBase in the AddDynamic binding
    PickupSphere->OnComponentBeginOverlap.AddDynamic(this, &AWeaponBase::OnOverlapBegin);
	WeaponMesh = Cast<USkeletalMeshComponent>(GetDefaultSubobjectByName(TEXT("Mesh")));
}

// Called every frame
void AWeaponBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}


void AWeaponBase::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT("Overlap with weapon pickup"));	
	if (ABaseCharacter* Player = Cast<ABaseCharacter>(OtherActor))
	{
		// Call function on character to give weapon
		Player->AddWeapon(this);  // implement EquipWeapon in your character
		PickupSphere->SetHiddenInGame(true);
	}
}


void AWeaponBase::OnFire(FVector TargetPoint)
{
	// Implement firing logic here
	UE_LOG(LogTemp, Warning, TEXT("Weapon Fired!"));
	UParticleSystemComponent* PSC = UGameplayStatics::SpawnEmitterAttached(
		MuzzleFlash,                 // particle system
		WeaponMesh,                   // attach to this component
		TEXT("Muzzle"),        // socket name
		FVector::ZeroVector,         // location offset
		FRotator::ZeroRotator,       // rotation offset
		EAttachLocation::SnapToTarget, // attach rules
		true                         // auto destroy
	);

	if (PSC)
	{
        // ... rest of your code remains unchanged ...
		UE_LOG(LogTemp, Warning, TEXT("Muzzle Flash Spawned"));	
		PSC->SetWorldScale3D(FVector(0.05f));
	}

	FVector MuzzleLocation = WeaponMesh->GetSocketLocation("Muzzle");

	FRotator SpawnRotation = (TargetPoint - MuzzleLocation).Rotation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = GetInstigator();

	GetWorld()->SpawnActor<ABulletBase>(
		BulletClass,
		MuzzleLocation,
		SpawnRotation,
		SpawnParams
	);
}