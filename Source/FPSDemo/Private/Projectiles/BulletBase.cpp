#include "Projectiles/BulletBase.h"
#include "Components/SphereComponent.h" // Add this include
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ABulletBase::ABulletBase()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
    CollisionComp->InitSphereRadius(2.f);
	CollisionComp->SetHiddenInGame(false);
	CollisionComp->SetGenerateOverlapEvents(false);
    CollisionComp->SetCollisionProfileName(TEXT("Projectile"));
    //CollisionComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
   

    RootComponent = CollisionComp;
    CollisionComp->OnComponentHit.AddDynamic(this, &ABulletBase::OnHit);

   /* BulletMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BulletMesh"));
    BulletMesh->SetupAttachment(RootComponent);
    BulletMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BulletMesh->SetRelativeScale3D(FVector(0.01f));*/

    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
    ProjectileMovement->UpdatedComponent = CollisionComp;
    ProjectileMovement->InitialSpeed = 40000.f;
    ProjectileMovement->MaxSpeed = 50000.f;
    ProjectileMovement->bRotationFollowsVelocity = true;
    ProjectileMovement->bShouldBounce = false;

    InitialLifeSpan = 10.0f;

    static ConstructorHelpers::FObjectFinder<UStaticMesh> MeshAsset(TEXT("/Engine/EditorMeshes/ArcadeEditorSphere.ArcadeEditorSphere"));
    if (MeshAsset.Succeeded())
    {
        //BulletMesh->SetStaticMesh(MeshAsset.Object);
    }
}

// Called when the game starts or when spawned
void ABulletBase::BeginPlay()
{
    Super::BeginPlay();
    if (AActor* MyOwner = GetOwner())
    {        
        UE_LOG(LogTemp, Warning, TEXT("MyInstigatorOWner %s"), *MyOwner->GetName());
        CollisionComp->IgnoreActorWhenMoving(MyOwner, true);
    }

    if (APawn* MyInstigator = GetInstigator())
    {
        UE_LOG(LogTemp, Warning, TEXT("MyInstigator: %s"), *MyInstigator->GetName());


        CollisionComp->IgnoreActorWhenMoving(MyInstigator, true);
    }
}

// Called every frame
void ABulletBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

}

void ABulletBase::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, FVector NormalImpulse,
    const FHitResult& Hit)
{
	// print debug message
	UE_LOG(LogTemp, Warning, TEXT("Bullet hit something: %s"), *GetName());
    if (!OtherActor || OtherActor == this || !OtherComp)
        return;

    UE_LOG(LogTemp, Warning, TEXT("Hit actor: %s"), *OtherActor->GetName());

    // explosion FX
    if (ExplosionFX)
    {
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionFX, Hit.ImpactPoint);
    }


    // spawn decal
    if (HitDecal) {
        UGameplayStatics::SpawnDecalAtLocation(GetWorld(), HitDecal,
            FVector(5.f), Hit.ImpactPoint,
            Hit.ImpactNormal.Rotation(), 10.0f);
    }

    Destroy();
}


void ABulletBase::InitFromData(UBulletData* InData, FVector FireDestination)
{
    Data = InData;
    if (Data)
    {
        if (Data->ExplosionFX)
        {
            ExplosionFX = Data->ExplosionFX;
        }
        if (Data->HitDecal)
        {
            HitDecal = Data->HitDecal;
        }
    }

    FireTowards(FireDestination);
}


void ABulletBase::FireTowards(const FVector& Target)
{
    FVector Start = GetActorLocation();
    FVector Dir = (Target - Start).GetSafeNormal();
	SetActorRelativeRotation(Dir.Rotation());
    ProjectileMovement->SetVelocityInLocalSpace(FVector::ForwardVector * ProjectileMovement->InitialSpeed);
    ProjectileMovement->Activate(true);
}