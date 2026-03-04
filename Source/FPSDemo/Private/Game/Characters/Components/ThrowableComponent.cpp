
#include "Game/Characters/Components/ThrowableComponent.h"
#include "Game/Characters/Components/AnimationComponent.h"
#include "Game/Characters/Components/EquipComponent.h"
#include "Game/Characters/Components/ActionStateComponent.h"
#include "Game/Characters/BaseCharacter.h"
#include "Shared/Data/Items/ItemConfig.h"
#include "Game/Projectiles/ThrownProjectile.h"
#include "Game/Projectiles/ThrownProjectileFrag.h"
#include "Game/Projectiles/ThrownProjectileSmoke.h"
#include "Game/Projectiles/ThrownProjectileStun.h"
#include "Game/Projectiles/ThrownProjectileIncendiary.h"
#include "Shared/Data/Items/ThrowableConfig.h"
#include "Game/Characters/Components/ItemVisualComponent.h"
#include "Game/Characters/Components/InventoryComponent.h"

UThrowableComponent::UThrowableComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UThrowableComponent::Init()
{
	CharacterOwner = Cast<ABaseCharacter>(GetOwner());
	check(CharacterOwner);

	AnimComp = CharacterOwner->GetAnimationComponent();
	EquipComp = CharacterOwner->GetEquipComponent();
	ActionStateComp = CharacterOwner->GetActionStateComponent();

	check(AnimComp);
	check(EquipComp);
	check(ActionStateComp);
}

bool UThrowableComponent::CanStartThrow() const
{
	if (!IsEnabled())
	{
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

	return ActionStateComp->CanThrowNow();
}

void UThrowableComponent::RequestStartThrow()
{
	if (!IsEnabled())
	{
		return;
	}
	if (CharacterOwner->IsLocallyControlled())
	{
		if (!CanStartThrow()) {
			return;
		}
		AnimComp->PlayThrowNadeMontage(); // predictive play
	}

	if (CharacterOwner->HasAuthority())
	{
		HandleThrow();
	}
	else
	{
		ServerThrow();
	}
}

// Called by Server
void UThrowableComponent::HandleThrow()
{
	if (!CanStartThrow())
	{
		return;
	}

	if (!ActionStateComp->TrySetState(EActionState::Throwing))
	{
		UE_LOG(LogTemp, Warning, TEXT("UThrowableComponent::HandleThrow: Failed to set state to Throwing"));
		return;
	}

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
	if (!ActionStateComp->IsInState(EActionState::Throwing))
	{
		return;
	}
	ActionStateComp->TrySetState(EActionState::Idle);

	// compute launch velocity
	FVector LaunchVelocity = ComputeThrowVelocity();
	FVector StartPos = CharacterOwner->GetThrowableLocation();
	AThrownProjectile* ThrownProj = nullptr;

	const UItemConfig* ItemConf = EquipComp->GetActiveItemConfig();
	const UThrowableConfig* ThrowableConfig = Cast<UThrowableConfig>(ItemConf);
	if (!ThrowableConfig)
	{
		UE_LOG(LogTemp, Warning, TEXT("UThrowableComponent::FinishThrow: Active item is not a throwable"));
		return;
	}

	// remove one throwable from inventory
	UInventoryComponent* InvComp = CharacterOwner->GetInventoryComponent();
	check(InvComp);
	InvComp->RemoveItem(ThrowableConfig->Id);

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	SpawnParams.Instigator = Cast<APawn>(GetOwner());
	SpawnParams.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	if (ThrowableConfig->Id == EItemId::GRENADE_SMOKE) {
		ThrownProj = GetWorld()->SpawnActor<AThrownProjectileSmoke>(
			AThrownProjectileSmoke::StaticClass(),
			StartPos,
			FRotator::ZeroRotator, SpawnParams);
	}
	else if (ThrowableConfig->Id == EItemId::GRENADE_STUN) {
		ThrownProj = GetWorld()->SpawnActor<AThrownProjectileStun>(
			AThrownProjectileStun::StaticClass(),
			StartPos,
			FRotator::ZeroRotator, SpawnParams);
	}
	else if (ThrowableConfig->Id == EItemId::GRENADE_INCENDIARY) {
		ThrownProj = GetWorld()->SpawnActor<AThrownProjectileIncendiary>(
			AThrownProjectileIncendiary::StaticClass(),
			StartPos,
			FRotator::ZeroRotator, SpawnParams);
	}
	else {
		ThrownProj = GetWorld()->SpawnActor<AThrownProjectile>(
			AThrownProjectileFrag::StaticClass(),
			StartPos,
			FRotator::ZeroRotator, SpawnParams);
	}

	if (!ThrownProj)
	{
		UE_LOG(LogTemp, Warning, TEXT("UThrowableComponent::FinishThrow: Failed to spawn thrown projectile"));
		return;
	}
	ThrownProj->InitFromData(ThrowableConfig);
	ThrownProj->LaunchProjectile(LaunchVelocity, CharacterOwner);

	EquipComp->AutoSelectBestWeapon();
}

void UThrowableComponent::MulticastThrowAction_Implementation()
{
	// ignore local player
	if (CharacterOwner->IsLocallyControlled())
	{
		return;
	}
	AnimComp->PlayThrowNadeMontage();
}

void UThrowableComponent::ServerThrow_Implementation()
{
	HandleThrow();
}

FVector UThrowableComponent::ComputeThrowVelocity() const
{
	FVector EyeLoc;
	FRotator EyeRot;
	CharacterOwner->GetActorEyesViewPoint(EyeLoc, EyeRot);

	// 30 degrees upward
	EyeRot.Pitch += 30.0f;  // if it goes downward, change to += 30.0f
	const FVector Dir = EyeRot.Vector().GetSafeNormal();
	return Dir * GrenadeInitSpeed;
}

void UThrowableComponent::OnNadeRelease()
{
	UItemVisualComponent* ItemVisualComp = CharacterOwner->GetItemVisualComponent();
	check(ItemVisualComp);
	ItemVisualComp->HideItemVisual();
}

void UThrowableComponent::OnEnabledChanged(bool bNowEnabled)
{
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle_FinishThrow);
	if (!bNowEnabled) {
		if (ActionStateComp->IsInState(EActionState::Throwing))
		{
			ActionStateComp->TrySetState(EActionState::Idle);
		}
	}
}