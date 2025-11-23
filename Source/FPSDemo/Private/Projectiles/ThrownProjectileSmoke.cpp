// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectiles/ThrownProjectileSmoke.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"

void AThrownProjectileSmoke::OnExplode ()
{
	UE_LOG(LogTemp, Log, TEXT("AThrownProjectileSmoke::ExplodeNow"));
	FVector ImpactPoint = GetActorLocation();
	MulticastExplode(ImpactPoint);
	SetLifeSpan(5.0f);
}

void AThrownProjectileSmoke::MulticastExplode_Implementation(const FVector& ImpactPoint)
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
            ImpactPoint,
            Rotation
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

    WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}