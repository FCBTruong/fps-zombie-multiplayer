#include "Game/Projectiles/ThrownProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SphereComponent.h"
#include "Sound/SoundBase.h"
#include "Game/GameManager.h"
#include "Particles/ParticleSystem.h"
#include "Shared/Data/Items/ThrowableConfig.h"
#include "Shared/System/ItemsManager.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AThrownProjectile::AThrownProjectile()
{
    // root collision
    Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
    RootComponent = Collision;
    Collision->InitSphereRadius(3.f);
    Collision->SetCollisionProfileName(TEXT("Throw"));
    Collision->SetNotifyRigidBodyCollision(true);
    Collision->SetSimulatePhysics(false);
	Collision->IgnoreActorWhenMoving(GetInstigator(), true);

    // visual mesh
    WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    WeaponMesh->SetupAttachment(Collision);

    // movement
    Projectile = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
    Projectile->bAutoActivate = false;
    Projectile->bRotationFollowsVelocity = true;
    Projectile->bShouldBounce = true;
    Projectile->Bounciness = 0.5f;
    Projectile->Friction = 0.6f;
    //Projectile->BounceVelocityStopSimulatingThreshold = 100.f;
    Projectile->ProjectileGravityScale = 1.0f;
    Projectile->bForceSubStepping = true;
    Projectile->UpdatedComponent = Collision;

    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;
    SetReplicateMovement(false);
}

void AThrownProjectile::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);
    if (!bDidHit) {
        WeaponMesh->AddLocalRotation(FRotator(360.f * DeltaSeconds, 360.f * DeltaSeconds, 0));
    }
}

// Called when the game starts or when spawned
void AThrownProjectile::BeginPlay()
{
    Super::BeginPlay();
    Collision->OnComponentHit.AddDynamic(this, &AThrownProjectile::OnProjectileHit);
}

void AThrownProjectile::OnProjectileHit(
    UPrimitiveComponent* HitComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, FVector NormalImpulse,
    const FHitResult& Hit)
{
    if (bDidHit || bIsExploded)
    {
        return;
    }
    bDidHit = true;
}

void AThrownProjectile::InitFromData(const UThrowableConfig* InData)
{
    Data = InData;
    MulticastInitData(Data->Id);
}

void AThrownProjectile::MulticastInitData_Implementation(EItemId ItemId)
{
    if (!Data)   
    {
		auto Item = UItemsManager::Get(GetWorld())->GetItemById(ItemId);
        if (Item) {
            Data = Cast<UThrowableConfig>(Item);
		}
    }
   
    if (Data->StaticMesh) {
        WeaponMesh->SetStaticMesh(Data->StaticMesh);
    }
}

void AThrownProjectile::LaunchProjectile(FVector LaunchVelocity, AActor* InstigatorActor)
{

    Projectile->SetVelocityInLocalSpace(LaunchVelocity);
    Projectile->Activate(true);

    if (HasAuthority()) {
        GetWorldTimerManager().SetTimer(
            TimerHandle_Explode,
            this,
            &AThrownProjectile::ExplodeNow,
            3.5f,
            false
        );
    }
}

void AThrownProjectile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(AThrownProjectile, bIsExploded);
    DOREPLIFETIME(AThrownProjectile, bDidHit);
}

// Server function to handle explosion
void AThrownProjectile::ExplodeNow()
{
    if (bIsExploded)
    {
        return;
    }

    bIsExploded = true;
    Collision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	OnExplode();
}

void AThrownProjectile::MulticastExplode_Implementation(const FVector& Location)
{
    // base behavior (optional)
}

void AThrownProjectile::OnExplode() {

}