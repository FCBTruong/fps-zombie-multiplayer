// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/Projectiles/ThrownProjectileSmoke.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "Shared/Data/Items/ThrowableConfig.h"

void AThrownProjectileSmoke::OnExplode ()
{
	FVector ImpactPoint = GetActorLocation();
	MulticastExplode(ImpactPoint);
	SetLifeSpan(0.1f);
}

void AThrownProjectileSmoke::MulticastExplode_Implementation(const FVector& ImpactPoint)
{
	// ignore for dedicated server
    if (GetNetMode() == NM_DedicatedServer)
    {
        return;
    }

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