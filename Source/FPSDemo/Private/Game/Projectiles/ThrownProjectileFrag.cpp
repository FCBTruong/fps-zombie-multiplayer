// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/Projectiles/ThrownProjectileFrag.h"
#include "Kismet/GameplayStatics.h"
#include "Shared/Data/Items/ThrowableConfig.h"
#include "Game/GameManager.h"
#include "Shared/Data/GlobalDataAsset.h"

void AThrownProjectileFrag::BeginPlay()
{
    Super::BeginPlay();
    UE_LOG(LogTemp, Log, TEXT("AThrownProjectileFrag::BeginPlay"));

    // play sound fire in the hole
	UGameManager* GMR = UGameManager::Get(GetWorld());
    if (GMR && GMR->GlobalData && GMR->GlobalData->FireInTheHoleSound)
    {
        UGameplayStatics::PlaySoundAtLocation(this, GMR->GlobalData->FireInTheHoleSound, GetActorLocation());
	}
}

void AThrownProjectileFrag::OnExplode ()
{
    UE_LOG(LogTemp, Log, TEXT("AThrownProjectile::ExplodeNow"));

    FVector ImpactPoint = GetActorLocation();

    float BaseDamage = 300.f;
    TSubclassOf<UDamageType> DamageType = UDamageType::StaticClass();
    AController* InstigatorController = GetInstigatorController();

    if (!InstigatorController) {
        UE_LOG(LogTemp, Warning, TEXT("AThrownProjectile::ExplodeNow - No InstigatorController"));
    }
    float InnerRadius = 200.f;
    float OuterRadius = 400.f;
    // visualize
   /* DrawDebugSphere(GetWorld(), ImpactPoint, InnerRadius, 16, FColor::Red, false, 2.0f);
    DrawDebugSphere(GetWorld(), ImpactPoint, OuterRadius, 16, FColor::Yellow, false, 2.0f);*/

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

    UE_LOG(LogTemp, Log, TEXT("AThrownProjectile::ExplodeNow - Applied radial damage"));

    MulticastExplode(ImpactPoint);
    SetLifeSpan(0.1f);
}

void AThrownProjectileFrag::MulticastExplode_Implementation(const FVector& ImpactPoint)
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

    UE_LOG(LogTemp, Log, TEXT("AThrownProjectile::MulticastExplode_Implementation"));
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

    Collision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}