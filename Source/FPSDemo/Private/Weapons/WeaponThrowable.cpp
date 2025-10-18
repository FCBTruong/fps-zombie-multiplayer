// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/WeaponThrowable.h"


AWeaponThrowable::AWeaponThrowable() {
  
}

void AWeaponThrowable::OnActivate(const FVector& V)
{
    if (!Collision)
    {
        Collision = NewObject<USphereComponent>(this, TEXT("Collision"));
        Collision->InitSphereRadius(7.f);
        Collision->SetCollisionProfileName(TEXT("Projectile"));
        Collision->SetIsReplicated(true);           // if you need it on clients
        if (!RootComponent) {
            SetRootComponent(Collision);
        }
        else {
            Collision->SetupAttachment(RootComponent);
        }
        Collision->RegisterComponent();
    }

    if (!Projectile)
    {
        Projectile = NewObject<UProjectileMovementComponent>(this, TEXT("Projectile"));
        Projectile->UpdatedComponent = Collision;
        Projectile->InitialSpeed = 0.f;
        Projectile->MaxSpeed = 4000.f;
        Projectile->bRotationFollowsVelocity = true;
        Projectile->bShouldBounce = true;
        Projectile->Bounciness = 0.35f;
        Projectile->Friction = 0.2f;
        Projectile->BounceVelocityStopSimulatingThreshold = 100.f;
        Projectile->ProjectileGravityScale = 1.0f;
        Projectile->bAutoActivate = false;
        Projectile->RegisterComponent();
    }

    Projectile->StopMovementImmediately();
    Projectile->Velocity = V;
    Projectile->Activate(true);
}


