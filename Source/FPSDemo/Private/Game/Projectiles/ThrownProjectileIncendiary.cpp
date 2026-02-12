// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/Projectiles/ThrownProjectileIncendiary.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Shared/Data/Items/ThrowableConfig.h"


void AThrownProjectileIncendiary::OnExplode()
{
    UE_LOG(LogTemp, Log, TEXT("AThrownProjectileIncendiary::ExplodeNow"));
    FVector ImpactPoint = GetActorLocation();
    MulticastExplode(ImpactPoint);
    SetLifeSpan(6.0f);

    FTimerHandle DamageTimer;
    GetWorld()->GetTimerManager().SetTimer(
        DamageTimer,
        this,
        &AThrownProjectileIncendiary::DoDamage,
        0.1f,
        true
    );

    //DrawDebugSphere(
    //    GetWorld(),
    //    GetActorLocation(),
    //    Data->ExplosionRadius,
    //    16,                 // segments
    //    FColor::Red,
    //    false,              // bPersistentLines
    //    5.0f                // life time
    //);
}

void AThrownProjectileIncendiary::MulticastExplode_Implementation(const FVector& ImpactPoint)
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
    if (Data && Data->SmokeFX)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            Data->SmokeFX,
            ImpactPoint
        );
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

void AThrownProjectileIncendiary::DoDamage()
{
	UE_LOG(LogTemp, Log, TEXT("AThrownProjectileIncendiary::DoDamage - Applying radial damage"));   
    UGameplayStatics::ApplyRadialDamage(
        GetWorld(),
        Data->Damage,          
        GetActorLocation(),
        Data->ExplosionRadius,
        UDamageType::StaticClass(),
        TArray<AActor*>(),      // ignored actors
        this,
        GetInstigatorController(),
        true,                   // do full damage
        ECC_Visibility          // LOS check (blocked by walls)
    );
}