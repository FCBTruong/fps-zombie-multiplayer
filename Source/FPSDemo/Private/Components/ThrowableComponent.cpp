
#include "Components/ThrowableComponent.h"
#include "Components/AnimationComponent.h"
#include "Components/EquipComponent.h"
#include "Components/ActionStateComponent.h"
#include "Characters/BaseCharacter.h"
#include "Items/ItemConfig.h"
#include "Projectiles/ThrownProjectile.h"
#include "Projectiles/ThrownProjectileFrag.h"
#include "Projectiles/ThrownProjectileSmoke.h"
#include "Projectiles/ThrownProjectileStun.h"
#include "Projectiles/ThrownProjectileIncendiary.h"
#include "Items/ThrowableConfig.h"

UThrowableComponent::UThrowableComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UThrowableComponent::BeginPlay()
{
	Super::BeginPlay();
	CharacterOwner = Cast<ABaseCharacter>(GetOwner());
	if (CharacterOwner)
	{
		AnimComp = CharacterOwner->GetAnimationComponent();
		EquipComp = CharacterOwner->GetEquipComponent();
		ActionStateComp = CharacterOwner->GetActionStateComponent();
	}
}

bool UThrowableComponent::CanStartThrow() const
{
	if (!EquipComp) {
		return false;
	}
	// check equipped item is throwable
	const UItemConfig* ItemConf = EquipComp->GetActiveItemConfig();
	if (!ItemConf)
	{
		return false;
	}
	if (ItemConf->GetItemType() != EItemType::Throwable)
	{
		return false;
	}

	if (!ActionStateComp)
	{
		return false;
	}

	if (!ActionStateComp->CanThrowNow())
	{
		return false;
	}

	return true;
}

void UThrowableComponent::RequestStartThrow()
{
	if (CharacterOwner && CharacterOwner->IsLocallyControlled())
	{
		if (!CanStartThrow()) return;
		if (AnimComp) AnimComp->PlayThrowNadeMontage(); // predictive play
	}

	if (GetOwner()->HasAuthority())
	{
		HandleThrow();
	}
	else
	{
		ServerThrow();
	}
}


void UThrowableComponent::HandleThrow()
{
	if (!CanStartThrow())
	{
		return;
	}

	ActionStateComp->TrySetState(EActionState::Throwing);

	GetWorld()->GetTimerManager().SetTimer(
		TimerHandle_FinishThrow,
		this,
		&UThrowableComponent::FinishThrow,
		1.2f,
		false
	);


	MulticastThrowAction();
}

void UThrowableComponent::FinishThrow()
{
	// check if state is still throwing
	if (ActionStateComp->GetState() != EActionState::Throwing)
	{
		return;
	}
	ActionStateComp->TrySetState(EActionState::Idle);

	// compute launch velocity
	FVector LaunchVelocity = ComputeThrowVelocity();


	ABaseCharacter* Character = Cast<ABaseCharacter>(GetOwner());
	if (!Character)
	{
		return;
	}

	FVector StartPos = Character->GetThrowableLocation();
	AThrownProjectile* ThrownProj = nullptr;

	const UItemConfig* ItemConf = EquipComp->GetActiveItemConfig();
	const UThrowableConfig* ThrowableConfig = Cast<UThrowableConfig>(ItemConf);

	if (!ThrowableConfig)
	{
		return;
	}
	if (ThrowableConfig->Id == EItemId::GRENADE_SMOKE) {
		ThrownProj = GetWorld()->SpawnActor<AThrownProjectileSmoke>(
			AThrownProjectileSmoke::StaticClass(),
			StartPos,
			FRotator::ZeroRotator);
	}
	else if (ThrowableConfig->Id == EItemId::GRENADE_STUN) {
		ThrownProj = GetWorld()->SpawnActor<AThrownProjectileStun>(
			AThrownProjectileStun::StaticClass(),
			StartPos,
			FRotator::ZeroRotator);
	}
	else if (ThrowableConfig->Id == EItemId::GRENADE_INCENDIARY) {
		ThrownProj = GetWorld()->SpawnActor<AThrownProjectileIncendiary>(
			AThrownProjectileIncendiary::StaticClass(),
			StartPos,
			FRotator::ZeroRotator);
	}
	else {
		ThrownProj = GetWorld()->SpawnActor<AThrownProjectile>(
			AThrownProjectileFrag::StaticClass(),
			StartPos,
			FRotator::ZeroRotator);
	}
	if (ThrownProj) {
		ThrownProj->SetOwner(GetOwner());
		ThrownProj->SetInstigator(Cast<APawn>(GetOwner()));
		ThrownProj->InitFromData(ThrowableConfig);
		ThrownProj->LaunchProjectile(LaunchVelocity, Character);
	}
}

void UThrowableComponent::MulticastThrowAction_Implementation()
{
	// ignore local player
	if (CharacterOwner && CharacterOwner->IsLocallyControlled())
	{
		return;
	}

	if (AnimComp)
	{
		AnimComp->PlayThrowNadeMontage();
	}
}

void UThrowableComponent::ServerThrow_Implementation()
{
	HandleThrow();
}

FVector UThrowableComponent::ComputeThrowVelocity() const
{
	if (!CharacterOwner) return FVector::ZeroVector;

	FVector EyeLoc;
	FRotator EyeRot;
	CharacterOwner->GetActorEyesViewPoint(EyeLoc, EyeRot);

	// 30 degrees upward
	EyeRot.Pitch += 30.0f;  // if it goes downward, change to += 30.0f

	const FVector Dir = EyeRot.Vector().GetSafeNormal();
	return Dir * GrenadeInitSpeed;
}