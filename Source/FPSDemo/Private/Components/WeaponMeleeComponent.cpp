// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/WeaponMeleeComponent.h"
#include "Components/ActionStateComponent.h"
#include "Components/ItemVisualComponent.h"
#include "Characters/BaseCharacter.h"
#include "Game/ItemsManager.h"
#include "Items/ItemConfig.h"
#include "Items/MeleeConfig.h"
#include "GameConstants.h"
#include "Damage/MyPointDamageEvent.h"
#include "Damage/MyDamageType.h"
#include "DrawDebugHelpers.h"
#include "Game/GameManager.h"
#include "Game/GlobalDataAsset.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"

UWeaponMeleeComponent::UWeaponMeleeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UWeaponMeleeComponent::Initialize(
	UActionStateComponent* InAction,
	UItemVisualComponent* InVisual
)
{
	ActionStateComp = InAction;
	VisualComp = InVisual;
	Character = Cast<ABaseCharacter>(GetOwner());
}

void UWeaponMeleeComponent::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Log, TEXT("UWeaponMeleeComponent::BeginPlay called"));
	Character = Cast<ABaseCharacter>(GetOwner());
}

void UWeaponMeleeComponent::RequestMeleeAttack(int32 AttackIndex)
{
	UE_LOG(LogTemp, Log, TEXT("RequestMeleeAttack called with AttackIndex: %d"), AttackIndex);
	if (!Character || !Character->IsAlive())
		return;

	if (IsOwningClient() && VisualComp)
	{
		if (!CanMeleeNow())
			return;
		ActionStateComp->TrySetState(EActionState::Melee);

		PredictMeleeHitFX();

		if (MeleeConfig) {
			if (AttackIndex == FGameConstants::MELEE_ATTACK_INDEX_PRIMARY) {
				if (MeleeConfig->Attack1Montage) {
					VisualComp->PlayMeleeAttack(MeleeConfig->Attack1Montage);
				}
			}
			else {
				if (MeleeConfig->Attack2Montage) {
					VisualComp->PlayMeleeAttack(MeleeConfig->Attack2Montage);
				}
			}
		}
	}

	if (Character->HasAuthority())
	{
		StartMelee_ServerAuth(AttackIndex);
	}
	else
	{
		ServerStartMelee(AttackIndex);
	}
}

void UWeaponMeleeComponent::ServerStartMelee_Implementation(int32 AttackIndex)
{
	StartMelee_ServerAuth(AttackIndex);
}

void UWeaponMeleeComponent::StartMelee_ServerAuth(int32 AttackIndex)
{
	if (!CanMeleeNow())
		return;

	// Set server state once
	ActionStateComp->TrySetState(EActionState::Melee);

	// Play montage for everyone (you can skip owning client here if you want)
	MulticastPlayMelee(AttackIndex);

	// Delay the trace by 1 second (hit timing)
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(MeleeTraceTimer);

		PerformMeleeTrace(AttackIndex);
		/*FTimerDelegate Delegate;
		Delegate.BindUObject(this, &UWeaponMeleeComponent::PerformMeleeTrace, AttackIndex);

		World->GetTimerManager().SetTimer(
			MeleeTraceTimer,
			Delegate,
			0.1f,
			false
		);*/
	}
}

void UWeaponMeleeComponent::MulticastPlayMelee_Implementation(int32 AttackIndex)
{
	UE_LOG(LogTemp, Log, TEXT("MulticastPlayMelee called with AttackIndex: %d"), AttackIndex);
	if (!Character)
		return;
	
	// Skip owning client if already played
	if (IsOwningClient())
		return;

	if (VisualComp)
	{
		if (MeleeConfig) {
			if (AttackIndex == FGameConstants::MELEE_ATTACK_INDEX_PRIMARY) {
				if (MeleeConfig->Attack1Montage) {
					VisualComp->PlayMeleeAttack(MeleeConfig->Attack1Montage);
				}
			}
			else {
				if (MeleeConfig->Attack2Montage) {
					VisualComp->PlayMeleeAttack(MeleeConfig->Attack2Montage);
				}
			}
		}
	}
}


void UWeaponMeleeComponent::PerformMeleeTrace(int32 AttackIndex)
{
	UE_LOG(LogTemp, Log, TEXT("PerformMeleeTrace called for AttackIndex: %d"), AttackIndex);

	FHitResult Hit;
	if (!DoMeleeSweep(Hit, MeleeRange, MeleeRadius))
	{
		ActionStateComp->TrySetState(EActionState::Idle);
		return;
	}
	if (AActor* Target = Hit.GetActor())
	{
		FMyPointDamageEvent DamageEvent;
		DamageEvent.DamageTypeClass = UMyDamageType::StaticClass();
		DamageEvent.WeaponID = MeleeConfig->Id;

		FName HitBoneName = Hit.BoneName;

		float Damage = MeleeConfig->Damage;

		static const TSet<FName> HeadBones = {
			TEXT("head"),
			TEXT("Head"),
			TEXT("neck_01")
		};

		if (HeadBones.Contains(HitBoneName))
		{
			Damage *= 4.0f;              // x4 headshot
			DamageEvent.bIsHeadshot = true;
		}

		Target->TakeDamage(
			Damage,
			DamageEvent,
			Character->GetController(),
			Character
		);
	}

	ActionStateComp->TrySetState(EActionState::Idle);
}



bool UWeaponMeleeComponent::CanMeleeNow() const
{
	if (!ActionStateComp)
		return false;

	if (!MeleeConfig) {
		return false;
	}

	if (!ActionStateComp->IsIdle())
		return false;
	return true;
}

bool UWeaponMeleeComponent::IsOwningClient() const
{
	return Character && Character->IsLocallyControlled();
}

void UWeaponMeleeComponent::HandleActiveItemChanged(EItemId MeleeId)
{
	if (MeleeId == EItemId::NONE)
	{
		MeleeConfig = nullptr;
		return;
	}
	MeleeConfig = Cast<UMeleeConfig>(
		UItemsManager::Get(GetWorld())->GetItemById(MeleeId)
	);
}

bool UWeaponMeleeComponent::DoMeleeSweep(
	FHitResult& OutHit,
	float Range,
	float Radius
) const
{
	if (!Character || !GetWorld())
		return false;

	FVector Start;
	FRotator ViewRot;
	Character->GetActorEyesViewPoint(Start, ViewRot);

	FVector End = Start + ViewRot.Vector() * Range;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Character);

	return GetWorld()->SweepSingleByChannel(
		OutHit,
		Start,
		End,
		FQuat::Identity,
		ECC_Pawn,
		FCollisionShape::MakeSphere(Radius),
		Params
	);
}

void UWeaponMeleeComponent::PredictMeleeHitFX()
{
	if (!IsOwningClient())
		return;

	FHitResult Hit;
	if (DoMeleeSweep(Hit, MeleeRange, MeleeRadius))
	{
		PlayHitFX_Local(Hit.ImpactPoint, Hit.ImpactNormal);
	}
}

void UWeaponMeleeComponent::PlayHitFX_Local(
	const FVector& ImpactPoint,
	const FVector& ImpactNormal
)
{
	UGameManager* GameManager = UGameManager::Get(GetWorld());
	if (!GameManager)
		return;

	UGlobalDataAsset* GlobalData = GameManager->GlobalData;
	UE_LOG(LogTemp, Log, TEXT("PlayHitFX_Local called at Point: %s, Normal: %s"),
		*ImpactPoint.ToString(), *ImpactNormal.ToString());
	if (GlobalData->MeleeHitFx) {
		/*auto Fx = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			GlobalData->MeleeHitFx,
			ImpactPoint,
			ImpactNormal.Rotation()
		);
		if (Fx)
		{
			Fx->SetWorldScale3D(FVector(0.02f));
		}
		Fx->SetFirstPersonPrimitiveType(EFirstPersonPrimitiveType::FirstPerson);*/

		UParticleSystemComponent* PSC = UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			GlobalData->MeleeHitFx,
			ImpactPoint,
			ImpactNormal.Rotation(),
			FVector(0.05f),
			true
		);

		if (PSC)
		{
			UE_LOG(LogTemp, Warning, TEXT("Muzzle Flash Spawned"));
			PSC->SetFirstPersonPrimitiveType(EFirstPersonPrimitiveType::FirstPerson);

		}
	}

	if (GlobalData->MeleeImpactBodySound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			GetWorld(),
			GlobalData->MeleeImpactBodySound,
			ImpactPoint
		);
	}
}
