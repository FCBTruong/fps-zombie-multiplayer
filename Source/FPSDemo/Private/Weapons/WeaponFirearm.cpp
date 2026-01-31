// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/WeaponFirearm.h"
#include "Projectiles/BulletBase.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Controllers/MyPlayerController.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Character.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Items/ItemConfig.h"
#include "Items/FirearmConfig.h"
#include "Game/GlobalDataAsset.h"
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
	UE_LOG(LogTemp, Warning, TEXT("Weapon Fired!"));
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
			UE_LOG(LogTemp, Warning, TEXT("Muzzle Flash Spawned"));
			PSC->SetWorldScale3D(FVector(0.15f));

			if (bIsFpsView) {
				PSC->SetFirstPersonPrimitiveType(EFirstPersonPrimitiveType::FirstPerson);
			}
		}
		else {
			UE_LOG(LogTemp, Warning, TEXT("Failed to spawn Muzzle Flash"));
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
		UNiagaraComponent* Trail =
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				GetWorld(),                               // World
				FC->BulletTrailNS,           // Niagara System
				MuzzleLocation,                         // Location (Start)
				FRotator::ZeroRotator,
				FVector::OneVector,
				true,   // AutoDestroy
				false    // AutoActivate
			);

		if (!Trail) return;

		// log start and end points
		Trail->SetVectorParameter(TEXT("MuzzlePosition"), MuzzleLocation);
		TArray<FVector> ImpactPositions;
		ImpactPositions.Add(TargetPoint);

		UNiagaraDataInterfaceArrayFunctionLibrary::SetNiagaraArrayVector(
			Trail,
			FName("User.ImpactPositions"),
			ImpactPositions
		);

		Trail->Activate(true);
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

		UE_LOG(LogTemp, Warning, TEXT("Spawned Niagara Muzzle Flash"));
	}
	
	if (WeaponMesh) {

		if (FC && FC->FireAnimation) // FireAnimation: UAnimSequenceBase* (or UAnimationAsset*)
		{
			UE_LOG(LogTemp, Warning, TEXT("Playing Fire Animation (no montage)"));

			// This switches the mesh to Single Node mode and plays the asset
			WeaponMesh->PlayAnimation(FC->FireAnimation, /*bLoop*/ false);
		}
	}
}

void AWeaponFirearm::PlayOutOfAmmoSound()
{
	const UFirearmConfig* FC = Cast<UFirearmConfig>(Config);
	if (!FC) {
		return;
	}
	/*if (FC->OutOfAmmoSFX)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FC->OutOfAmmoSFX, GetActorLocation());
	}*/
}

void AWeaponFirearm::PlayReloadSound()
{
	const UFirearmConfig* FC = Cast<UFirearmConfig>(Config);
	if (!FC) {
		return;
	}
	/*if (FC->ReloadSFX)
	{
		UGameplayStatics::PlaySoundAtLocation(this, FC->ReloadSFX, GetActorLocation());
	}*/
}

void AWeaponFirearm::ApplyConfig()
{
	Super::ApplyConfig();
	const UFirearmConfig* FC = Cast<UFirearmConfig>(Config);
	if (!FC) {
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Applying Weapon Data in Firearm"));
	if (WeaponMesh) {
		WeaponMesh->HideBoneByName(FName("b_gun_mag"), EPhysBodyOp::PBO_None);
	}
	// Get Mag Mesh
	if (MagMesh && Config && FC->MagMesh)
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
	UE_LOG(LogTemp, Warning, TEXT("AWeaponFirearm::SetViewFps called with bFps=%d"), bFps);
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