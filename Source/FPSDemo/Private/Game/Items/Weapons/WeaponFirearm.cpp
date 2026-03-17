// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/Items/Weapons/WeaponFirearm.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Shared/Data/Items/ItemConfig.h"
#include "Shared/Data/Items/FirearmConfig.h"
#include "NiagaraDataInterfaceArrayFunctionLibrary.h"
#include "Game/Characters/Components/WeaponFireComponent.h"
#include "Game/Projectiles/BulletData.h"
#include "Components/DecalComponent.h"
#include "Game/Characters/BaseCharacter.h"
#include "Game/GameManager.h"
#include "Shared/Data/GlobalDataAsset.h"

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

void AWeaponFirearm::OnFire(const TArray<FBulletImpactData>& Impacts, FVector TargetPoint)
{
	UE_LOG(LogTemp, Log, TEXT("WeaponFirearm::OnFire called with %d impacts"), Impacts.Num());
	FVector MuzzleLocation = WeaponMesh->GetSocketLocation("Muzzle");

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

	// Handle bullet impacts vfx (e.g., spawn decals, effects)
	if (!FC->BulletData) {
		return;
	}

	FVector ShotDirection = (TargetPoint - MuzzleLocation).GetSafeNormal();
	for (const FBulletImpactData& Impact : Impacts)
	{
		if (IsValid(Impact.ImpactActor) && Impact.ImpactActor->IsA<APawn>()) {
			if (FC->BulletData->HitBodySound) {
				UGameplayStatics::PlaySoundAtLocation(this, FC->BulletData->HitBodySound, Impact.ImpactPoint);
			}

			ABaseCharacter* HitCharacter = Cast<ABaseCharacter>(Impact.ImpactActor);
			if (HitCharacter) {
				HitCharacter->PlayBloodFx(Impact.ImpactPoint, Impact.ImpactNormal);
			}

			TraceBehindPawnAndSpawnBloodDecal(Impact.ImpactPoint, ShotDirection);
			continue;
		}

		// effect for non-pawn surfaces
		if (FC->BulletData->HitSurfaceSound) {
			UGameplayStatics::PlaySoundAtLocation(this, FC->BulletData->HitSurfaceSound, Impact.ImpactPoint);
		}
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), FC->BulletData->ExplosionFX, Impact.ImpactPoint);
		const float LifeTime = 10.0f;
		UDecalComponent* Decal = UGameplayStatics::SpawnDecalAtLocation(GetWorld(), 
			FC->BulletData->HitDecal,
			FVector(5.f), Impact.ImpactPoint,
			Impact.ImpactNormal.Rotation(), LifeTime);
		Decal->SetFadeScreenSize(0.001f);

		if (Decal)
		{
			const float FadeDuration = 3.0f;   // last 3 seconds
			const float FadeStartDelay = FMath::Max(0.f, LifeTime - FadeDuration);

			Decal->SetFadeOut(FadeStartDelay, FadeDuration, true);
		}
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	APawn* Shooter = GetInstigator();
	if (!Shooter) {
		return;
	}
	SpawnParams.Instigator = Shooter;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

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

void AWeaponFirearm::TraceBehindPawnAndSpawnBloodDecal(FVector ImpactPoint, FVector Dir)
{
	if (Dir.IsNearlyZero())
		Dir = GetActorForwardVector();

	const float TraceDistance = 300.f;
	const float StartOffset = 5.f; // start just past the pawn hit so we don't re-hit the pawn

	const FVector TraceStart = ImpactPoint + Dir * StartOffset;
	const FVector TraceEnd = TraceStart + Dir * TraceDistance;

	FCollisionQueryParams Params(SCENE_QUERY_STAT(BloodBehindPawnTrace), /*bTraceComplex*/ true);
	Params.AddIgnoredActor(this);
	
	FHitResult BehindHit;
	const bool bHit = GetWorld()->LineTraceSingleByObjectType(
		BehindHit,
		TraceStart,
		TraceEnd,
		FCollisionObjectQueryParams(ECC_WorldStatic),
		Params
	);

	if (!bHit || !BehindHit.bBlockingHit)
		return;

	// Spawn blood decal only on non-pawn surfaces behind the pawn
	if (BehindHit.GetActor())
	{
		UGameManager* GM = UGameManager::Get(this);
		UGlobalDataAsset* DataAsset = GM ? GM->GlobalData : nullptr;
		UMaterialInterface* BloodDecal = DataAsset ? DataAsset->BloodDecal : nullptr;

		if (!BloodDecal)
			return;
		// Random size 30-40 and add a small random roll (rotation) to avoid identical decals

		const float RandomSize = FMath::RandRange(40.f, 50.f);

		// Base rotation from surface normal
		FRotator DecalRot = BehindHit.ImpactNormal.Rotation();

		// Add random roll around the normal (this is what changes "spin" on the surface)
		DecalRot.Roll = FMath::RandRange(0.f, 360.f);

		const float LifeTime = 25.0f;
		UDecalComponent* Decal = UGameplayStatics::SpawnDecalAtLocation(
			GetWorld(),
			BloodDecal,
			FVector(RandomSize),
			BehindHit.ImpactPoint,
			DecalRot,
			LifeTime
		);

		if (Decal)
		{
			const float FadeDuration = 5.0f;   // last 3 seconds
			const float FadeStartDelay = FMath::Max(0.f, LifeTime - FadeDuration);

			Decal->SetFadeOut(FadeStartDelay, FadeDuration, true);
		}


		if (Decal)
		{
			Decal->SetFadeScreenSize(0.001f);
		}
	}
}
