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
	if (Data->MuzzleFX) {
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
			PSC->SetWorldScale3D(FVector(0.15f));


			if (bIsFpsView) {
				if (USceneCaptureComponent2D* Capture = ViewmodelCapture.Get()) // ViewmodelCapture is TWeakObjectPtr
				{
					Capture->ShowOnlyComponent(PSC);
					// If needed:
					// Capture->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
				}

				PSC->SetOwnerNoSee(true);
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
	SpawnParams.Instigator = Shooter;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	UE_LOG(LogTemp, Warning, TEXT("Shooter: %s"), *Shooter->GetName());

	FVector StartPointBullet = MuzzleLocation;
	if (bCustomStart) {
		StartPointBullet = StartPoint;
	}

	/*ABulletBase* Bullet = GetWorld()->SpawnActor<ABulletBase>(
		ABulletBase::StaticClass(),
		StartPointBullet,
		FRotator::ZeroRotator,
		SpawnParams
	);
	Bullet->InitFromData(Data->BulletData, TargetPoint, ViewmodelCapture);*/

	if (bIsFpsView) {
		UGameManager* GMR = GetWorld()->GetGameInstance()->GetSubsystem<UGameManager>();
		if (!GMR || !GMR->GlobalData || !GMR->GlobalData->BulletTrailNS) return;

		UNiagaraComponent* Trail =
			UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				GetWorld(),                               // World
				GMR->GlobalData->BulletTrailNS,           // Niagara System
				StartPointBullet,                         // Location (Start)
				FRotator::ZeroRotator,
				FVector::OneVector,
				true,   // AutoDestroy
				false    // AutoActivate
			);

		if (!Trail) return;

		// log start and end points
		UE_LOG(LogTemp, Warning, TEXT("Bullet Trail Start: %s, End: %s"), *StartPointBullet.ToString(), *TargetPoint.ToString());

		Trail->SetVectorParameter(TEXT("BeamEnd"), StartPointBullet);
		Trail->SetVectorParameter(TEXT("BeamStart"), TargetPoint);

		Trail->Activate(true);
	}

	// Play sound
	if (Data->FireSFX)
	{
		UGameplayStatics::PlaySoundAtLocation(this, Data->FireSFX, GetActorLocation());
	}


	//if (Data && Data->MuzzleFlashFX && WeaponMesh)
	//{
	//	UNiagaraComponent* Niagara = UNiagaraFunctionLibrary::SpawnSystemAttached(
	//		Data->MuzzleFlashFX,
	//		WeaponMesh,
	//		FName(TEXT("Muzzle")),
	//		FVector::ZeroVector,
	//		FRotator::ZeroRotator,
	//		FVector(1.f),
	//		EAttachLocation::SnapToTarget,
	//		/*bAutoDestroy*/   true,
	//		/*PoolingMethod*/  ENCPoolMethod::None,
	//		/*bAutoActivate*/  true,
	//		/*bPreCullCheck*/  true
	//	);
	//}
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
	UE_LOG(LogTemp, Warning, TEXT("Applying Weapon Data in Firearm"));
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
			if (!ViewmodelCapture.IsValid()) {
				return;
			}
			ViewmodelCapture->ShowOnlyComponents.AddUnique(MagMesh);
			MagMesh->SetOwnerNoSee(true);
		}
		else {
			MagMesh->SetOwnerNoSee(false);
		}
	}
}