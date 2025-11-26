// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/WeaponFirearm.h"
#include "Projectiles/BulletBase.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Controllers/MyPlayerController.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"
#include "NiagaraFunctionLibrary.h"

AWeaponFirearm::AWeaponFirearm()
{
	MagMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MagMesh"));
	AttachMagToDefault();
}

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
		PSC->SetWorldScale3D(FVector(0.5f));
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

	if (Data->MuzzleFlashFX) {
		UNiagaraComponent* Niagara = UNiagaraFunctionLibrary::SpawnSystemAttached(
			Data->MuzzleFlashFX,
			WeaponMesh,
			TEXT("Muzzle"),
			FVector::ZeroVector,
			FRotator::ZeroRotator,
			EAttachLocation::SnapToTarget,
			true,  // auto activate
			true   // auto destroy
		);
	}
}


void AWeaponFirearm::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	// current ammo, max ammo replication
	DOREPLIFETIME(AWeaponFirearm, CurrentAmmo);
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
				PC->PlayerUI->UpdateAmmo(CurrentAmmo, GetMaxAmmo());
			}
		}
	}
}


void AWeaponFirearm::ConsumeAmmo(int Amount)
{
	if (GetOwner()->HasAuthority()) // only server modifies ammo
	{
		CurrentAmmo = FMath::Max(0, CurrentAmmo - Amount);
		UE_LOG(LogTemp, Warning, TEXT("Ammo consumed. CurrentAmmo: %d"), CurrentAmmo);
	}
}

void AWeaponFirearm::SetCurrentAmmo(int NewCurrentAmmo)
{
	CurrentAmmo = FMath::Clamp(NewCurrentAmmo, 0, GetMaxAmmo());
	UE_LOG(LogTemp, Warning, TEXT("CurrentAmmo set to: %d"), CurrentAmmo);
}

bool AWeaponFirearm::HasAmmoInClip() const
{
	if (CurrentAmmo > 0)
	{
		return true;
	}
	return false;
}

void AWeaponFirearm::PlayOutOfAmmoSound()
{
	if (Data->OutOfAmmoSFX)
	{
		UGameplayStatics::PlaySoundAtLocation(this, Data->OutOfAmmoSFX, GetActorLocation());
	}
}

void AWeaponFirearm::PlayReloadSound()
{
	if (Data->ReloadSFX)
	{
		UGameplayStatics::PlaySoundAtLocation(this, Data->ReloadSFX, GetActorLocation());
	}
}

void AWeaponFirearm::ApplyWeaponData()
{
	Super::ApplyWeaponData();
	if (WeaponMesh) {
		WeaponMesh->HideBoneByName(FName("b_gun_mag"), EPhysBodyOp::PBO_None);
	}
	// Get Mag Mesh
	if (MagMesh && Data && Data->MagMesh)
	{
		MagMesh->SetStaticMesh(Data->MagMesh);

		ACharacter* Character = Cast<ACharacter>(GetOwner());
		if (Character && Character->IsLocallyControlled()) {
			MagMesh->SetCastShadow(false);
		}
	}
	CurrentAmmo = Data->MaxAmmoInClip;
}

void AWeaponFirearm::AttachMagToDefault()
{
	if (MagMesh)
	{
		MagMesh->AttachToComponent(
			WeaponMesh,
			FAttachmentTransformRules::SnapToTargetNotIncludingScale,
			TEXT("MagSocket")
		);
	}
}

int32 AWeaponFirearm::GetMaxAmmo() const
{
	return Data->MaxAmmoInClip;
}