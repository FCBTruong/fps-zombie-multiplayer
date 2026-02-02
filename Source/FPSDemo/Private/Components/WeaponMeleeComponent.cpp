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
#include "Damage/DamageHelpers.h"

UWeaponMeleeComponent::UWeaponMeleeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UWeaponMeleeComponent::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Log, TEXT("UWeaponMeleeComponent::BeginPlay called"));
	Character = Cast<ABaseCharacter>(GetOwner());

	if (Character) {
		ActionStateComp = Character->GetActionStateComponent();
		VisualComp = Character->GetItemVisualComponent();
	}
}

void UWeaponMeleeComponent::RequestMeleeAttack(int32 AttackIndex)
{
	if (!IsEnabled())
		return;

	UE_LOG(LogTemp, Log, TEXT("RequestMeleeAttack called with AttackIndex: %d"), AttackIndex);
	if (!Character || !Character->IsAlive())
		return;

	if (IsOwningClient() && VisualComp)
	{
		if (!CanMeleeNow())
			return;
		ActionStateComp->TrySetState(EActionState::Melee);

		GetWorld()->GetTimerManager().SetTimer(
			MeleeClientFxTimer,
			this,
			&UWeaponMeleeComponent::PredictMeleeHitFX,
			MeleeTraceDelay,
			false
		);

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
	if (!IsEnabled())
		return;
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

		//PerformMeleeTrace(AttackIndex);
		FTimerDelegate Delegate;
		Delegate.BindUObject(this, &UWeaponMeleeComponent::PerformMeleeTrace, AttackIndex);

		World->GetTimerManager().SetTimer(
			MeleeTraceTimer,
			Delegate,
			MeleeTraceDelay,
			false
		);
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

		FMyPointDamageEvent DamageEvent;
		DamageEvent.DamageTypeClass = UMyDamageType::StaticClass();
		DamageEvent.WeaponID = MeleeConfig->Id;

		FDamageApplyParams Params;
		Params.BaseDamage = MeleeConfig->Damage;
		Params.WeaponId = MeleeConfig->Id;
		Params.DamageTypeClass = UMyDamageType::StaticClass();
		Params.bEnableHeadshot = true;
		Params.HeadshotMultiplier = 4.f;
		Params.Hit = Hit;

		// log damage
		UE_LOG(LogTemp, Log, TEXT("Applying melee damage to Actor: %s, Damage: %f"),
			*Target->GetName(), Params.BaseDamage);

		DamageHelpers::ApplyMyPointDamage(
			Target,
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
		return false;
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
	if (!IsEnabled())
		return;

	if (MeleeId == EItemId::NONE)
	{
		MeleeConfig = nullptr;
		return;
	}
	MeleeConfig = Cast<UMeleeConfig>(
		UItemsManager::Get(GetWorld())->GetItemById(MeleeId)
	);
}

void UWeaponMeleeComponent::PredictMeleeHitFX()
{
	if (!IsOwningClient())
		return;

	TArray<FHitResult> Hits;
	if (DoMeleeSweepMulti(Hits, MeleeRange, MeleeRadius))
	{
		for (const FHitResult& H : Hits)
		{
			AActor* A = H.GetActor();
			if (!A) continue;

			if (!A->IsA<APawn>()) // only pawns
				continue;

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

bool UWeaponMeleeComponent::DoMeleeSweepMulti(
	TArray<FHitResult>& OutHits,
	float Range,
	float Radius
) const
{
	OutHits.Reset();

	if (!Character || !GetWorld())
		return false;

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