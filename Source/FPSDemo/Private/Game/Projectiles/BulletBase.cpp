#include "Game/Projectiles/BulletBase.h"
#include "Components/SphereComponent.h" // Add this include
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Game/Characters/BaseCharacter.h"
#include "NiagaraComponent.h"
#include "Game/GameManager.h"
#include "Shared/Data/GlobalDataAsset.h"
#include "Components/DecalComponent.h"

// Sets default values
ABulletBase::ABulletBase()
{
    PrimaryActorTick.bCanEverTick = false;

    CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
    CollisionComp->InitSphereRadius(1.f);
	CollisionComp->SetHiddenInGame(true);
	CollisionComp->SetGenerateOverlapEvents(false);
    CollisionComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    RootComponent = CollisionComp;
    CollisionComp->OnComponentHit.AddDynamic(this, &ABulletBase::OnHit);

    ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
    ProjectileMovement->UpdatedComponent = CollisionComp;
    ProjectileMovement->InitialSpeed = 30000.f;
    ProjectileMovement->MaxSpeed = 30000.f;
    ProjectileMovement->bRotationFollowsVelocity = true;
    ProjectileMovement->bShouldBounce = false;
    ProjectileMovement->bAutoActivate = false;
    ProjectileMovement->ProjectileGravityScale = 0.0f;

    InitialLifeSpan = 2.0f;
}

// Called when the game starts or when spawned
void ABulletBase::BeginPlay()
{
    Super::BeginPlay();
    CollisionComp->SetCollisionProfileName(TEXT("Bullet"));
    if (AActor* MyOwner = GetOwner())
    {        
        CollisionComp->IgnoreActorWhenMoving(MyOwner, true);
    }

    if (APawn* MyInstigator = GetInstigator())
    {
        CollisionComp->IgnoreActorWhenMoving(MyInstigator, true);
    }

	ProjectileMovement->Activate(true);
}

void ABulletBase::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, FVector NormalImpulse,
    const FHitResult& Hit)
{
	// print debug message
    if (!OtherActor || OtherActor == this || !OtherComp)
        return;

    // explosion FX
    if ((!OtherActor->IsA<APawn>()) && ExplosionFX)
    {
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ExplosionFX, Hit.ImpactPoint);
    }

    // spawn decal
    if (OtherActor->IsA<APawn>() == false) {
        if (HitDecal) {
			const float LifeTime = 10.0f;
            UDecalComponent* Decal = UGameplayStatics::SpawnDecalAtLocation(GetWorld(), HitDecal,
                FVector(5.f), Hit.ImpactPoint,
                Hit.ImpactNormal.Rotation(), LifeTime);
            Decal->SetFadeScreenSize(0.001f);

            if (Decal)
            {
                const float FadeDuration = 3.0f;   // last 3 seconds
                const float FadeStartDelay = FMath::Max(0.f, LifeTime - FadeDuration);

                Decal->SetFadeOut(FadeStartDelay, FadeDuration, true);
            }
        }

        if (HitSurfaceSound) {
            UGameplayStatics::PlaySoundAtLocation(this, HitSurfaceSound, Hit.ImpactPoint);
        }
    }
    else {
        if (Data->HitBodySound) {
            UGameplayStatics::PlaySoundAtLocation(this, Data->HitBodySound, Hit.ImpactPoint);
        }

        // blood fx
        ABaseCharacter* Enemy = Cast<ABaseCharacter>(OtherActor);
        if (Enemy)
        {
            Enemy->PlayBloodFx(Hit.ImpactPoint, Hit.ImpactNormal);
        }
		// continue to trace to find the surface behind the character and spawn blood fx there
		TraceBehindPawnAndSpawnBloodDecal(Hit);
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
        if (Data->HitSurfaceSound)
        {
            HitSurfaceSound = Data->HitSurfaceSound;
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

void ABulletBase::TraceBehindPawnAndSpawnBloodDecal(const FHitResult& PawnHit)
{
    if (!HitDecal) // replace HitDecal with BloodDecal if you have a separate one
        return;

    FVector Dir = ProjectileMovement ? ProjectileMovement->Velocity.GetSafeNormal() : GetActorForwardVector();
    if (Dir.IsNearlyZero())
        Dir = GetActorForwardVector();

    const float TraceDistance = 300.f;
    const float StartOffset = 5.f; // start just past the pawn hit so we don't re-hit the pawn

    const FVector TraceStart = PawnHit.ImpactPoint + Dir * StartOffset;
    const FVector TraceEnd = TraceStart + Dir * TraceDistance;

    FCollisionQueryParams Params(SCENE_QUERY_STAT(BloodBehindPawnTrace), /*bTraceComplex*/ true);
    Params.AddIgnoredActor(this);
    if (AActor* OwnerActor = GetOwner()) Params.AddIgnoredActor(OwnerActor);
    if (APawn* Inst = GetInstigator())   Params.AddIgnoredActor(Inst);
    if (AActor* PawnActor = PawnHit.GetActor()) Params.AddIgnoredActor(PawnActor);

    FHitResult BehindHit;
    const bool bHit = GetWorld()->LineTraceSingleByChannel(
        BehindHit,
        TraceStart,
        TraceEnd,
        ECC_Visibility, // or your bullet channel
        Params
    );

    if (!bHit || !BehindHit.bBlockingHit)
        return;

    // Spawn blood decal only on non-pawn surfaces behind the pawn
    if (BehindHit.GetActor())
    {
        UGameManager* GM = UGameManager::Get(this);
        UGlobalDataAsset* DataAsset = GM ? GM->GlobalData : nullptr;
        UMaterialInterface* BloodDecal = DataAsset ? DataAsset->BloodDecal : nullptr;

        if (!BloodDecal)
            return;
        // Random size 30-40 and add a small random roll (rotation) to avoid identical decals

        const float RandomSize = FMath::RandRange(40.f, 50.f);

        // Base rotation from surface normal
        FRotator DecalRot = BehindHit.ImpactNormal.Rotation();

        // Add random roll around the normal (this is what changes "spin" on the surface)
        DecalRot.Roll = FMath::RandRange(0.f, 360.f);

        const float LifeTime = 25.0f;
        UDecalComponent* Decal = UGameplayStatics::SpawnDecalAtLocation(
            GetWorld(),
            BloodDecal,
            FVector(RandomSize),
            BehindHit.ImpactPoint,
            DecalRot,
            LifeTime
        );

        if (Decal)
        {
            const float FadeDuration = 5.0f;   // last 3 seconds
            const float FadeStartDelay = FMath::Max(0.f, LifeTime - FadeDuration);

            Decal->SetFadeOut(FadeStartDelay, FadeDuration, true);
        }


        if (Decal)
        {
            Decal->SetFadeScreenSize(0.001f);
        }
    }
}
