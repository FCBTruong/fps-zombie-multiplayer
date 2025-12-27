// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/WeaponMeleeComponent.h"
#include "Components/EquipComponent.h"
#include "Components/ActionStateComponent.h"
#include "Components/ItemVisualComponent.h"
#include "Characters/BaseCharacter.h"

UWeaponMeleeComponent::UWeaponMeleeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UWeaponMeleeComponent::Initialize(
	UEquipComponent* InEquip,
	UActionStateComponent* InAction,
	UItemVisualComponent* InVisual
)
{
	EquipComp = InEquip;
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
		VisualComp->PlayMeleeAttack(AttackIndex);
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

		FTimerDelegate Delegate;
		Delegate.BindUObject(this, &UWeaponMeleeComponent::PerformMeleeTrace, AttackIndex);

		World->GetTimerManager().SetTimer(
			MeleeTraceTimer,
			Delegate,
			1.0f,
			false
		);
	}
}

void UWeaponMeleeComponent::MulticastPlayMelee_Implementation(int32 AttackIndex)
{
	UE_LOG(LogTemp, Log, TEXT("MulticastPlayMelee called with AttackIndex: %d"), AttackIndex);
	if (!Character)
		return;

	if (VisualComp)
	{
		VisualComp->PlayMeleeAttack(AttackIndex);
	}
}

void UWeaponMeleeComponent::PerformMeleeTrace(int32 AttackIndex)
{
	UE_LOG(LogTemp, Log, TEXT("PerformMeleeTrace called for AttackIndex: %d"), AttackIndex);

	FVector Start;
	FRotator ViewRot;
	Character->GetActorEyesViewPoint(Start, ViewRot);

	float MeleeRange = 150.f;
	FVector End = Start + ViewRot.Vector() * MeleeRange;

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(Character);

	// Sphere sweep prevents precision exploits
	FCollisionShape Shape = FCollisionShape::MakeSphere(25.f);

	if (GetWorld()->SweepSingleByChannel(
		Hit,
		Start,
		End,
		FQuat::Identity,
		ECC_Pawn,
		Shape,
		Params))
	{
		if (AActor* Target = Hit.GetActor())
		{
			/*FPointDamageEvent DamageEvent;
			DamageEvent.DamageTypeClass = UMyDamageType::StaticClass();

			Target->TakeDamage(
				Melee.Damage,
				DamageEvent,
				Character->GetController(),
				Character
			);*/
		}
	}
	ActionStateComp->TrySetState(EActionState::Idle);
}


bool UWeaponMeleeComponent::CanMeleeNow() const
{
	if (!ActionStateComp)
		return false;

	if (!ActionStateComp->IsIdle())
		return false;
	return true;
}

bool UWeaponMeleeComponent::IsOwningClient() const
{
	return Character && Character->IsLocallyControlled();
}
