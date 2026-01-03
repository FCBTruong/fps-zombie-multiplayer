#include "Projectiles/BulletBase.h"
#include "Components/SphereComponent.h" // Add this include
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Characters/BaseCharacter.h"
#include "NiagaraComponent.h"
#include "Game/GameManager.h"
#include "Game/GlobalDataAsset.h"

// Sets default values
ABulletBase::ABulletBase()
{
    // Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
    PrimaryActorTick.bCanEverTick = true;

    CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
    CollisionComp->InitSphereRadius(1.f);
	CollisionComp->SetHiddenInGame(true);
	CollisionComp->SetGenerateOverlapEvents(false);
    CollisionComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
   

    RootComponent = CollisionComp;
    CollisionComp->OnComponentHit.AddDynamic(this, &ABulletBase::OnHit);

   /* BulletMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BulletMesh"));
    BulletMesh->SetupAttachment(RootComponent);
    BulletMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BulletMesh->SetRelativeScale3D(FVector(0.01f));*/

    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
    ProjectileMovement->UpdatedComponent = CollisionComp;
    ProjectileMovement->InitialSpeed = 30000.f;
    ProjectileMovement->MaxSpeed = 30000.f;
    ProjectileMovement->bRotationFollowsVelocity = true;
    ProjectileMovement->bShouldBounce = false;
    ProjectileMovement->bAutoActivate = false;
    ProjectileMovement->ProjectileGravityScale = 0.0f;

    InitialLifeSpan = 2.0f;

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
    CollisionComp->SetCollisionProfileName(TEXT("Bullet"));
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

	ProjectileMovement->Activate(true);
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
    if ((!OtherActor->IsA<APawn>()) && ExplosionFX)
    {
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionFX, Hit.ImpactPoint);
    }


    // spawn decal
    if (HitDecal) {
        UGameplayStatics::SpawnDecalAtLocation(GetWorld(), HitDecal,
            FVector(5.f), Hit.ImpactPoint,
            Hit.ImpactNormal.Rotation(), 10.0f);
    }

    // blood fx
    ABaseCharacter* Enemy = Cast<ABaseCharacter>(OtherActor);
    if (Enemy)
    {
        Enemy->PlayBloodFx(Hit.ImpactPoint, Hit.ImpactNormal);
    }

    SetLifeSpan(0.5f);
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
	UGameManager* GMR = UGameManager::Get(GetWorld());  
    UNiagaraSystem* BulletTrailNS = GMR->GlobalData->BulletTrailNS;
    if (BulletTrailNS)
    {
        UNiagaraComponent* Trail = UNiagaraFunctionLibrary::SpawnSystemAttached(
            BulletTrailNS,
            CollisionComp,
            NAME_None,
            FVector::ZeroVector,
            FRotator::ZeroRotator,
            EAttachLocation::SnapToTarget,
            true
        );
    }
}