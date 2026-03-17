// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/Characters/Components/WeaponMeleeComponent.h"
#include "Game/Characters/Components/ActionStateComponent.h"
#include "Game/Characters/Components/ItemVisualComponent.h"
#include "Game/Characters/BaseCharacter.h"
#include "Shared/System/ItemsManager.h"
#include "Shared/Data/Items/ItemConfig.h"
#include "Shared/Data/Items/MeleeConfig.h"
#include "GameConstants.h"
#include "Game/Utils/Damage/MyDamageType.h"
#include "DrawDebugHelpers.h"
#include "Game/GameManager.h"
#include "Shared/Data/GlobalDataAsset.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "Game/Utils/Damage/DamageHelpers.h"
#include "GameFramework/GameStateBase.h"

UWeaponMeleeComponent::UWeaponMeleeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UWeaponMeleeComponent::Init()
{
	Character = Cast<ABaseCharacter>(GetOwner());
	check(Character);

	ActionStateComp = Character->GetActionStateComponent();
	check(ActionStateComp);

	VisualComp = Character->GetItemVisualComponent();
	check(VisualComp);
}

void UWeaponMeleeComponent::RequestMeleeAttack(int32 AttackIndex)
{
	if (!IsEnabled())
	{
		return;
	}
	if (!Character->IsAlive())
	{
		return;
	}

#if !UE_SERVER
	if (IsOwningClient())
	{
		if (!CanMeleeNow())
			return;

		GetWorld()->GetTimerManager().ClearTimer(MeleeTraceTimer);
		GetWorld()->GetTimerManager().SetTimer(
			MeleeClientFxTimer,
			this,
			&UWeaponMeleeComponent::PredictMeleeHitFX,
			MeleeTraceDelay,
			false
		);

		if (!Character->HasAuthority())
		{
			float Now = 0;
			if (const UWorld* World = GetWorld())
			{
				if (const AGameStateBase* GS = World->GetGameState())
				{
					Now = GS->GetServerWorldTimeSeconds();
				}
			}
			LastAttackTime = Now;
		}

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
#endif

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
	if (!IsEnabled())
	{
		return;
	}
	StartMelee_ServerAuth(AttackIndex);
}

void UWeaponMeleeComponent::StartMelee_ServerAuth(int32 AttackIndex)
{
	if (!CanMeleeNow())
	{
		return;
	}

	// Set server state once
	if (!ActionStateComp->TrySetState(EActionState::Melee))
	{
		return;
	}
	float Now = 0;
	if (const UWorld* World = GetWorld())
	{
		if (const AGameStateBase* GS = World->GetGameState())
		{
			Now = GS->GetServerWorldTimeSeconds();
		}
	}
	LastAttackTime = Now;
	// Play montage for everyone (you can skip owning client here if you want)
	MulticastPlayMelee(AttackIndex);

	//PerformMeleeTrace(AttackIndex);
	FTimerDelegate Delegate;
	Delegate.BindUObject(this, &UWeaponMeleeComponent::PerformMeleeTrace, AttackIndex);
	GetWorld()->GetTimerManager().ClearTimer(MeleeTraceTimer);
	GetWorld()->GetTimerManager().SetTimer(
		MeleeTraceTimer,
		Delegate,
		MeleeTraceDelay,
		false
	);
}

void UWeaponMeleeComponent::MulticastPlayMelee_Implementation(int32 AttackIndex)
{
	// Skip owning client if already played
	if (IsOwningClient())
	{
		return;
	}

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

void UWeaponMeleeComponent::PerformMeleeTrace(int32 AttackIndex)
{
	if (!MeleeConfig) {
		return;
	}

	if (!ActionStateComp->IsInState(EActionState::Melee))
	{
		return;
	}

	TArray<FHitResult> Hits;
	if (!DoMeleeSweepMulti(Hits, MeleeRange, MeleeRadius))
	{
		ActionStateComp->TrySetState(EActionState::Idle);
		return;
	}

	// Prioritize closest hits first
	Hits.Sort([](const FHitResult& A, const FHitResult& B)
		{
			return A.Distance < B.Distance;
		});

	TSet<TWeakObjectPtr<AActor>> DamagedActors;
	for (const FHitResult& Hit : Hits)
	{
		AActor* Target = Hit.GetActor();
		if (!Target || Target == Character)
			continue;

		// Prevent multi-hit on same actor from multiple hit results
		if (DamagedActors.Contains(Target))
			continue;

		DamagedActors.Add(Target);

		FDamageApplyParams Params;
		Params.BaseDamage = MeleeConfig->Damage;
		Params.WeaponId = MeleeConfig->Id;
		Params.DamageTypeClass = UMyDamageType::StaticClass();
		Params.bEnableHeadshot = true;
		Params.Hit = Hit;

		DamageHelpers::ApplyMyPointDamage(
			Params,
			Character->GetController(),
			Character
		);

		// limit max number of actors hit per swing (cleave limit)
		if (DamagedActors.Num() >= 2) break;
	}
	ActionStateComp->TrySetState(EActionState::Idle);
}

bool UWeaponMeleeComponent::CanMeleeNow() const
{
	if (!IsEnabled())
	{
		return false;
	}
	if (!MeleeConfig) {
		return false;
	}

	if (!ActionStateComp->IsIdle())
	{
		return false;
	}

	float Now = 0;
	if (const UWorld* World = GetWorld())
	{
		if (const AGameStateBase* GS = World->GetGameState())
		{
			Now = GS->GetServerWorldTimeSeconds();
		}
	}

	if (Now - LastAttackTime < MeleeConfig->Interval)
	{
		return false;
	}
	return true;
}

bool UWeaponMeleeComponent::IsOwningClient() const
{
	return Character->IsLocallyControlled();
}

void UWeaponMeleeComponent::HandleActiveItemChanged(EItemId MeleeId)
{
	if (MeleeId == EItemId::NONE)
	{
		MeleeConfig = nullptr;
		return;
	}

	UItemsManager* ItemsManager = UItemsManager::Get(GetWorld());
	MeleeConfig = Cast<UMeleeConfig>(ItemsManager->GetItemById(MeleeId));
}

void UWeaponMeleeComponent::PredictMeleeHitFX()
{
	if (!IsOwningClient())
	{
		return;
	}

	TArray<FHitResult> Hits;
	TSet<TWeakObjectPtr<AActor>> HitActors; // prevent multiple FX on same actor from multiple hit results

	if (DoMeleeSweepMulti(Hits, MeleeRange, MeleeRadius))
	{
		for (const FHitResult& H : Hits)
		{
			AActor* A = H.GetActor();
			if (!A) continue;

			if (HitActors.Contains(A))
			{
				continue;
			}
		
			APawn* Pawn = Cast<APawn>(A);
			if (!IsValid(Pawn))
			{
				continue;
			}

			ABaseCharacter* HitChar = Cast<ABaseCharacter>(Pawn);
			if (HitChar && !HitChar->IsAlive())
			{
				continue;
			}
			HitActors.Add(A);
			PlayHitFX_Local(H.ImpactPoint, H.ImpactNormal);
		}
	}
}

void UWeaponMeleeComponent::PlayHitFX_Local(
	const FVector& ImpactPoint,
	const FVector& ImpactNormal
)
{
	UGameManager* GameManager = UGameManager::Get(GetWorld());
	UGlobalDataAsset* GlobalData = GameManager->GlobalData;

	if (GlobalData->MeleeHitFx) {
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

bool UWeaponMeleeComponent::DoMeleeSweepMulti(
	TArray<FHitResult>& OutHits,
	float Range,
	float Radius
) const
{
	OutHits.Reset();
	FVector Start;
	FRotator ViewRot;
	Character->GetActorEyesViewPoint(Start, ViewRot);
	const FVector End = Start + ViewRot.Vector() * Range;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(MeleeSweep), false, Character);
	Params.AddIgnoredActor(Character);

	const bool bHitAny = GetWorld()->SweepMultiByChannel(
		OutHits,
		Start,
		End,
		FQuat::Identity,
		ECC_Pawn,
		FCollisionShape::MakeSphere(Radius),
		Params
	);

	return bHitAny && OutHits.Num() > 0;
}