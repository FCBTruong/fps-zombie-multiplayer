// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/Projectiles/ThrownProjectileIncendiary.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Shared/Data/Items/ThrowableConfig.h"
#include "Components/DecalComponent.h"

void AThrownProjectileIncendiary::OnExplode()
{
    FVector ImpactPoint = GetActorLocation();
    MulticastExplode(ImpactPoint);
    SetLifeSpan(LifeTime);

    FTimerHandle DamageTimer;
    GetWorld()->GetTimerManager().SetTimer(
        DamageTimer,
        this,
        &AThrownProjectileIncendiary::DoDamage,
        0.1f,
        true
    );

  /*  DrawDebugSphere(
        GetWorld(),
        GetActorLocation(),
        Data->ExplosionRadius,
        16,                 // segments
        FColor::Red,
        false,              // bPersistentLines
        LifeTime                // life time
    ); */
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

    if (Data && Data->ExplosionSFX)
    {
        UGameplayStatics::PlaySoundAtLocation(this, Data->ExplosionSFX, ImpactPoint);
    }

    if (Data && Data->SmokeFX)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            Data->SmokeFX,
            ImpactPoint
        );
    }

    if (Data && Data->DecalMat)
    {
        auto Decal = UGameplayStatics::SpawnDecalAtLocation(GetWorld(), Data->DecalMat,
            FVector(Data->DecalSize), ImpactPoint,
            Rotation, LifeTime);

        if (Decal)
        {
            const float FadeDuration = 1.0f;   // last 3 seconds
            const float FadeStartDelay = FMath::Max(0.f, LifeTime - FadeDuration);

            Decal->SetFadeOut(FadeStartDelay, FadeDuration, true);
        }
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