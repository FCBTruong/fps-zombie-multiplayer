// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/WeaponFirearm.h"
#include "Projectiles/BulletBase.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"

void AWeaponFirearm::OnFire(FVector TargetPoint)
{
	// Implement firing logic here
	UE_LOG(LogTemp, Warning, TEXT("Weapon Fired!"));
	UParticleSystemComponent* PSC = UGameplayStatics::SpawnEmitterAttached(
		Data->Config.MuzzleFX,                 // particle system
		WeaponMesh,                   // attach to this component
		TEXT("Muzzle"),        // socket name
		FVector::ZeroVector,         // location offset
		FRotator::ZeroRotator,       // rotation offset
		EAttachLocation::SnapToTarget, // attach rules
		true                         // auto destroy
	);

	if (PSC)
	{
		// ... rest of your code remains unchanged ...
		UE_LOG(LogTemp, Warning, TEXT("Muzzle Flash Spawned"));
		PSC->SetWorldScale3D(FVector(0.05f));
	}
	else {
		UE_LOG(LogTemp, Warning, TEXT("Failed to spawn Muzzle Flash"));
	}

	FVector MuzzleLocation = WeaponMesh->GetSocketLocation("Muzzle");

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	APawn* Shooter = GetInstigator();
	SpawnParams.Instigator = Shooter;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	UE_LOG(LogTemp, Warning, TEXT("Shooter: %s"), *Shooter->GetName());

	ABulletBase* Bullet = GetWorld()->SpawnActor<ABulletBase>(
		ABulletBase::StaticClass(),
		MuzzleLocation,
		FRotator::ZeroRotator,
		SpawnParams
	);
	Bullet->InitFromData(Data->BulletData, TargetPoint);

	// Play sound
	if (Data->Config.FireSFX)
	{
		UGameplayStatics::PlaySoundAtLocation(this, Data->Config.FireSFX, GetActorLocation());
	}
}
