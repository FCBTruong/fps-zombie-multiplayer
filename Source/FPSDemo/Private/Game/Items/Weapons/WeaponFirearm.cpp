// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/Items/Weapons/WeaponFirearm.h"
#include "Game/Projectiles/BulletBase.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Shared/Data/Items/ItemConfig.h"
#include "Shared/Data/Items/FirearmConfig.h"
#include "NiagaraDataInterfaceArrayFunctionLibrary.h"

AWeaponFirearm::AWeaponFirearm()
{
	MagMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MagMesh"));
	MagMesh->SetupAttachment(WeaponMesh, TEXT("MagSocket"));
}

void AWeaponFirearm::BeginPlay()
{
	Super::BeginPlay();
	AttachMagToDefault();
}

void AWeaponFirearm::OnFire(const FVector& TargetPoint, bool bCustomStart, const FVector& StartPoint)
{
	// Implement firing logic here
	const UFirearmConfig* FC = Cast<UFirearmConfig>(Config);
	if (!FC) {
		return;
	}
	if (FC->MuzzleFX) {
		UParticleSystemComponent* PSC = UGameplayStatics::SpawnEmitterAttached(
			FC->MuzzleFX,                 // particle system
			WeaponMesh,                   // attach to this component
			TEXT("Muzzle"),        // socket name
			FVector::ZeroVector,         // location offset
			FRotator::ZeroRotator,       // rotation offset
			EAttachLocation::SnapToTarget, // attach rules
			true                         // auto destroy
		);

		if (PSC)
		{
			PSC->SetWorldScale3D(FVector(0.15f));
			if (bIsFpsView) {
				PSC->SetFirstPersonPrimitiveType(EFirstPersonPrimitiveType::FirstPerson);
			}
		}
	}

	FVector MuzzleLocation = WeaponMesh->GetSocketLocation("Muzzle");

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	APawn* Shooter = GetInstigator();
	if (!Shooter) {
		return;
	}
	SpawnParams.Instigator = Shooter;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	FVector StartPointBullet = MuzzleLocation;
	if (bCustomStart) {
		StartPointBullet = StartPoint;
	}

	ABulletBase* Bullet = GetWorld()->SpawnActor<ABulletBase>(
		ABulletBase::StaticClass(),
		StartPointBullet,
		FRotator::ZeroRotator,
		SpawnParams
	);
	Bullet->InitFromData(FC->BulletData, TargetPoint);

	if (FC->BulletTrailNS) {
		FVector TrailStart = MuzzleLocation + (TargetPoint - MuzzleLocation).GetSafeNormal() * 20.f;
		UNiagaraComponent* Trail =
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				GetWorld(),                               // World
				FC->BulletTrailNS,           // Niagara System
				TrailStart,                         // Location (Start)
				FRotator::ZeroRotator,
				FVector::OneVector,
				true,   // AutoDestroy
				false    // AutoActivate
			);

		if (Trail) {
			Trail->SetVectorParameter(TEXT("MuzzlePosition"), TrailStart);
			TArray<FVector> ImpactPositions;
			ImpactPositions.Add(TargetPoint);

			UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector(
				Trail,
				FName("User.ImpactPositions"),
				ImpactPositions
			);
			Trail->Activate(true);
		}
	}

	// Play sound
	if (FC->FireSFX)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FC->FireSFX, GetActorLocation());
	}

	if (FC->CasingClass)
	{
		// current weapon location
		FVector CasingEjectLocation = WeaponMesh->GetSocketLocation("CasingEject");
		FRotator CasingEjectRotation = WeaponMesh->GetSocketRotation("CasingEject");

		CasingEjectRotation.Pitch += FMath::RandRange(-10.f, 10.f);
		CasingEjectRotation.Yaw += FMath::RandRange(-15.f, 15.f);
		CasingEjectRotation.Roll += FMath::RandRange(-30.f, 30.f);
		GetWorld()->SpawnActor<AActor>(
			FC->CasingClass,
			CasingEjectLocation,
			CasingEjectRotation,
			SpawnParams
		);
	}

	if (FC->MuzzleFlashFX)
	{
		UNiagaraComponent* MuzzleFlash =
			UNiagaraFunctionLibrary::SpawnSystemAttached(
				FC->MuzzleFlashFX,
				WeaponMesh,
				TEXT("Muzzle"),
				FVector::ZeroVector,
				FRotator(180, -90, 0),
				EAttachLocation::SnapToTarget,
				true,
				true
			);
		
		if (bIsFpsView) {
			MuzzleFlash->SetFirstPersonPrimitiveType(EFirstPersonPrimitiveType::FirstPerson);
		}
	}
	
	if (FC->FireAnimation) {	
		WeaponMesh->PlayAnimation(FC->FireAnimation, /*bLoop*/ false);
	}
}

void AWeaponFirearm::PlayOutOfAmmoSound()
{
}

void AWeaponFirearm::PlayReloadSound()
{
}

void AWeaponFirearm::ApplyConfig()
{
	Super::ApplyConfig();
	const UFirearmConfig* FC = Cast<UFirearmConfig>(Config);
	if (!FC) {
		return;
	}

	WeaponMesh->HideBoneByName(FName("b_gun_mag"), EPhysBodyOp::PBO_None);
	// Get Mag Mesh
	if (MagMesh && FC->MagMesh)
	{
		MagMesh->SetStaticMesh(FC->MagMesh);
	}
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
	const UFirearmConfig* FC = Cast<UFirearmConfig>(Config);
	if (!FC) {
		return 0;
	}
	return FC->MaxAmmoInClip;
}

void AWeaponFirearm::Destroyed()
{
	Super::Destroyed();
	if (MagMesh)
	{
		MagMesh->DestroyComponent();
	}
}

void AWeaponFirearm::SetViewFps(bool bFps)
{
	Super::SetViewFps(bFps);
	if (MagMesh)
	{
		if (bFps) {
			MagMesh->SetFirstPersonPrimitiveType(EFirstPersonPrimitiveType::FirstPerson);
		}
		else {
			MagMesh->SetFirstPersonPrimitiveType(EFirstPersonPrimitiveType::None);
		}
	}
}