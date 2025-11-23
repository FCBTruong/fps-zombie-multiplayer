#include "Projectiles/ThrownProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SphereComponent.h"
#include "Sound/SoundBase.h"
#include "Game/GameManager.h"
#include "Particles/ParticleSystem.h"

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

    // visual mesh
    WeaponMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    WeaponMesh->SetupAttachment(Collision);

    // movement
    Projectile = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Throw"));
    Projectile->bAutoActivate = false;
    Projectile->bRotationFollowsVelocity = true;
    Projectile->bShouldBounce = true;
    Projectile->Bounciness = 0.5f;
    Projectile->Friction = 0.6f;
    Projectile->BounceVelocityStopSimulatingThreshold = 100.f;
    Projectile->ProjectileGravityScale = 1.0f;
    Projectile->bForceSubStepping = true;
    Projectile->UpdatedComponent = Collision;

    bReplicates = true;
    SetReplicateMovement(true);
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
	UE_LOG(LogTemp, Log, TEXT("AThrownProjectile::OnProjectileHit"));
}

void AThrownProjectile::InitFromData(UWeaponData* InData)
{
    Data = InData;
    if (!HasAuthority()) {
		UE_LOG(LogTemp, Log, TEXT("AThrownProjectile::InitFromData - not Authority"));
	}
	UE_LOG(LogTemp, Log, TEXT("AThrownProjectile::InitFromData - Data %s"), Data ? *Data->GetName() : TEXT("null"));
   
    MulticastInitData(Data->Id);
}


void AThrownProjectile::MulticastInitData_Implementation(EItemId ItemId)
{
    // Load Data from ItemId
    UGameManager* GMR = GetWorld()->GetGameInstance()->GetSubsystem<UGameManager>();

    if (!Data)   
    {
        Data = Cast<UWeaponData>(GMR->GetItemDataById(ItemId));
    }
    UE_LOG(LogTemp, Log, TEXT("AThrownProjectile::MulticastInitData_Implementation - ItemId %d"), static_cast<int32>(ItemId));
   
    
    if (Data->StaticMesh) {
        WeaponMesh->SetStaticMesh(Data->StaticMesh);
    }
}

void AThrownProjectile::LaunchProjectile(FVector LaunchVelocity, AActor* InstigatorActor)
{
    if (Projectile)
    {
        Projectile->SetVelocityInLocalSpace(LaunchVelocity);
        Projectile->Activate(true);

        if (HasAuthority()) {
            GetWorldTimerManager().SetTimer(
                TimerHandle_Explode,
                this,
                &AThrownProjectile::ExplodeNow,
                5.0f,
                false
            );
        }
    }
}


void AThrownProjectile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
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