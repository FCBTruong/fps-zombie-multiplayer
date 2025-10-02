#include "BulletBase.h"
#include "Components/SphereComponent.h" // Add this include
#include "GameFramework/ProjectileMovementComponent.h"

// Sets default values
ABulletBase::ABulletBase()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
    CollisionComp->InitSphereRadius(5.f);
    CollisionComp->SetCollisionProfileName("Projectile");
    RootComponent = CollisionComp;

    BulletMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BulletMesh"));
    BulletMesh->SetupAttachment(RootComponent);
    BulletMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
    ProjectileMovement->UpdatedComponent = CollisionComp;

    InitialLifeSpan = 3.0f;
}

// Called when the game starts or when spawned
void ABulletBase::BeginPlay()
{
    Super::BeginPlay();
    if (AActor* MyOwner = GetOwner())
    {        
        UE_LOG(LogTemp, Warning, TEXT("MyInstigatorOWner"));
        CollisionComp->IgnoreActorWhenMoving(MyOwner, true);
    }

    if (APawn* MyInstigator = GetInstigator())
    {
        UE_LOG(LogTemp, Warning, TEXT("MyInstigator"));

        CollisionComp->IgnoreActorWhenMoving(MyInstigator, true);
    }
}

// Called every frame
void ABulletBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

}
