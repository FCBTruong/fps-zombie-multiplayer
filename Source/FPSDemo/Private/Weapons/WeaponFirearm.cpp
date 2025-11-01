// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/WeaponFirearm.h"
#include "Projectiles/BulletBase.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Controllers/MyPlayerController.h"
#include "Net/UnrealNetwork.h"


void AWeaponFirearm::OnFire(FVector TargetPoint)
{
	// Implement firing logic here
	UE_LOG(LogTemp, Warning, TEXT("Weapon Fired!"));
	UParticleSystemComponent* PSC = UGameplayStatics::SpawnEmitterAttached(
		Data->MuzzleFX,                 // particle system
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
	if (Data->FireSFX)
	{
		UGameplayStatics::PlaySoundAtLocation(this, Data->FireSFX, GetActorLocation());
	}
}


void AWeaponFirearm::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	// current ammo, max ammo replication
	DOREPLIFETIME(AWeaponFirearm, CurrentAmmo);
	DOREPLIFETIME(AWeaponFirearm, MaxAmmo);
}

void AWeaponFirearm::OnRep_CurrentAmmo()
{
	// Handle client-side logic when CurrentAmmo is updated
	UE_LOG(LogTemp, Warning, TEXT("CurrentAmmo replicated: %d"), CurrentAmmo);

	if (auto* Pawn = Cast<APawn>(GetOwner()))
	{
		UE_LOG(LogTemp, Warning, TEXT("Owner Pawn found: %s"), *Pawn->GetName());
		if (auto* PC = Cast<AMyPlayerController>(Pawn->GetController()))
		{
			if (PC->PlayerUI)
			{
				PC->PlayerUI->UpdateAmmo(CurrentAmmo, MaxAmmo);
			}
		}
	}
}

void AWeaponFirearm::OnRep_MaxAmmo()
{
	// Handle client-side logic when MaxAmmo is updated
	UE_LOG(LogTemp, Warning, TEXT("MaxAmmo replicated: %d"), MaxAmmo);
}

void AWeaponFirearm::ConsumeAmmo(int Amount)
{
	if (GetOwner()->HasAuthority()) // only server modifies ammo
	{
		CurrentAmmo = FMath::Max(0, CurrentAmmo - Amount);
		UE_LOG(LogTemp, Warning, TEXT("Ammo consumed. CurrentAmmo: %d"), CurrentAmmo);
	}
}