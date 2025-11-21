#include "Projectiles/AThrownProjectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SphereComponent.h"
#include "Sound/SoundBase.h"
#include "Game/GameManager.h"
#include "Particles/ParticleSystem.h"

// Sets default values
AAThrownProjectile::AAThrownProjectile()
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
void AAThrownProjectile::BeginPlay()
{
    Super::BeginPlay();
    Collision->OnComponentHit.AddDynamic(this, &AAThrownProjectile::OnProjectileHit);
}

void AAThrownProjectile::OnProjectileHit(
    UPrimitiveComponent* HitComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, FVector NormalImpulse,
    const FHitResult& Hit)
{
	UE_LOG(LogTemp, Log, TEXT("AAThrownProjectile::OnProjectileHit"));
}

void AAThrownProjectile::InitFromData(UWeaponData* InData)
{
    Data = InData;
    if (!HasAuthority()) {
		UE_LOG(LogTemp, Log, TEXT("AAThrownProjectile::InitFromData - not Authority"));
	}
	UE_LOG(LogTemp, Log, TEXT("AAThrownProjectile::InitFromData - Data %s"), Data ? *Data->GetName() : TEXT("null"));
   
    MulticastInitData(Data->Id);
}


void AAThrownProjectile::MulticastInitData_Implementation(EItemId ItemId)
{
    // Load Data from ItemId
    UGameManager* GMR = GetWorld()->GetGameInstance()->GetSubsystem<UGameManager>();

    if (!Data)   
    {
        Data = Cast<UWeaponData>(GMR->GetItemDataById(ItemId));
    }
    UE_LOG(LogTemp, Log, TEXT("AAThrownProjectile::MulticastInitData_Implementation - ItemId %d"), static_cast<int32>(ItemId));
   
    
    if (Data->StaticMesh) {
        WeaponMesh->SetStaticMesh(Data->StaticMesh);
    }
}

void AAThrownProjectile::LaunchProjectile(FVector LaunchVelocity, AActor* InstigatorActor)
{
    if (Projectile)
    {
        Projectile->SetVelocityInLocalSpace(LaunchVelocity);
        Projectile->Activate(true);

        if (HasAuthority()) {
            GetWorldTimerManager().SetTimer(
                TimerHandle_Explode,
                this,
                &AAThrownProjectile::ExplodeNow,
                5.0f,
                false
            );
        }
    }
}
void AAThrownProjectile::MulticastExplode_Implementation(const FVector& ImpactPoint)
{
    FHitResult Hit;
    FVector Start = GetActorLocation();
    FVector End = Start - FVector(0, 0, 300);
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(this);
	FRotator Rotation = FRotator::ZeroRotator;
    if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_WorldStatic, Params))
    {
       Rotation = Hit.ImpactNormal.Rotation();
    }
   
	UE_LOG(LogTemp, Log, TEXT("AAThrownProjectile::MulticastExplode_Implementation"));
    if (Data && Data->ExplosionFX)
    {
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Data->ExplosionFX,
            ImpactPoint, Rotation);
    }

    if (Data && Data->ExplosionSFX)
    {
        UGameplayStatics::PlaySoundAtLocation(this, Data->ExplosionSFX, ImpactPoint);
    }

    if (Data && Data->DecalMat)
    {
        UGameplayStatics::SpawnDecalAtLocation(GetWorld(), Data->DecalMat,
            FVector(Data->DecalSize), ImpactPoint,
            Rotation, Data->DecalLife);
    }

    // Stop movement
    if (Projectile)
    {
        Projectile->StopMovementImmediately();
        Projectile->Deactivate();
    }

    WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}


void AAThrownProjectile::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

// Server function to handle explosion
void AAThrownProjectile::ExplodeNow()
{
    if (bIsExploded)
        return;

    bIsExploded = true;
    UE_LOG(LogTemp, Log, TEXT("AAThrownProjectile::ExplodeNow"));

    FVector ImpactPoint = GetActorLocation();


    float BaseDamage = 300.f;
    TSubclassOf<UDamageType> DamageType = UDamageType::StaticClass();
    AController* InstigatorController = GetInstigatorController();
    float InnerRadius = 200.f;
    float OuterRadius = 400.f;
    // visualize
    DrawDebugSphere(GetWorld(), ImpactPoint, InnerRadius, 16, FColor::Red, false, 2.0f);
    DrawDebugSphere(GetWorld(), ImpactPoint, OuterRadius, 16, FColor::Yellow, false, 2.0f);

    UGameplayStatics::ApplyRadialDamageWithFalloff(
        GetWorld(),
        BaseDamage,             // full damage at inner radius
        10.f,                   // minimum damage at outer radius
        ImpactPoint,            // origin
        InnerRadius,                  // inner radius (100 cm = 1 m)
        OuterRadius,                  // outer radius (max reach)
        1.0f,                   // falloff exponent
        DamageType,
        TArray<AActor*>(),
        this,
        InstigatorController,
        ECC_Visibility
    );

    UE_LOG(LogTemp, Log, TEXT("AAThrownProjectile::ExplodeNow - Applied radial damage"));

    MulticastExplode(ImpactPoint);
    SetLifeSpan(0.1f);
}