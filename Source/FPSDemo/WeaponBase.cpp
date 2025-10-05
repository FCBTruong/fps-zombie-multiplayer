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
	PrimaryActorTick.bCanEverTick = false;

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	
	RootComponent = WeaponMesh;

	PickupSphere = CreateDefaultSubobject<USphereComponent>(TEXT("PickupSphere"));
	PickupSphere->SetupAttachment(WeaponMesh);

	PickupSphere->OnComponentBeginOverlap.AddDynamic(this, &AWeaponBase::OnOverlapBegin);
}

void AWeaponBase::PreInitializeComponents()
{
	Super::PreInitializeComponents();
	SetReplicates(true);

	WeaponMesh->SetSimulatePhysics(true);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	WeaponMesh->SetCollisionObjectType(ECC_PhysicsBody);
	WeaponMesh->SetCollisionResponseToAllChannels(ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

	if (PickupSphere)
	{
		PickupSphere->InitSphereRadius(100.f);
		PickupSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		PickupSphere->SetCollisionResponseToAllChannels(ECR_Ignore);
		PickupSphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	}
}

// Called when the game starts or when spawned
void AWeaponBase::BeginPlay()
{
	Super::BeginPlay();
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
		WeaponMesh->SetSimulatePhysics(false);
		WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		SetReplicateMovement(false);
		// Call function on character to give weapon
		Player->AddWeapon(this);  // implement EquipWeapon in your character
		PickupSphere->SetActive(false);
		SetInstigator(Player);
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
	APawn* Shooter = GetInstigator();
	SpawnParams.Instigator = Shooter;
	UE_LOG(LogTemp, Warning, TEXT("Shooter: %s"), *Shooter->GetName());


	ABulletBase* Bullet = GetWorld()->SpawnActor<ABulletBase>(
		BulletClass,
		MuzzleLocation,
		SpawnRotation,
		SpawnParams
	);
}

EWeaponTypes AWeaponBase::GetWeaponType()
{
	return EWeaponTypes::Firearm; // Example, change as needed
}

