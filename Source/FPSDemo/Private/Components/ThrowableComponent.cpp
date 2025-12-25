
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
	if (GetOwner()->HasAuthority())
	{
		HandleThrow();
	}
	else
	{
		if (!CanStartThrow())
		{
			return;
		}
		// play throw prep montage
		if (AnimComp)
		{
			AnimComp->PlayThrowNadeMontage();
		}
		ServerThrow();
	}
}


void UThrowableComponent::HandleThrow()
{
	if (!CanStartThrow())
	{
		return;
	}

	// compute launch velocity
	FVector LaunchVelocity = ComputeThrowVelocity();


	ABaseCharacter* Character = Cast<ABaseCharacter>(GetOwner());

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


	//FTimerHandle TimerHandle_FinishThrow;
	//GetWorld()->GetTimerManager().SetTimer(
	//	TimerHandle_FinishThrow,
	//	this,
	//	&UWeaponComponent::OnFinishedThrow,
	//	0.5f,
	//	false
	//);

	//// destroy current weapon and also remove it from array
	//ThrowablesArray.Remove(CurrentWeaponId);

	//MulticastThrowAction(LaunchVelocity);
}

void UThrowableComponent::ServerThrow_Implementation()
{
	HandleThrow();
}

FVector UThrowableComponent::ComputeThrowVelocity() const
{
	if (!CharacterOwner)
	{
		return FVector::ZeroVector;
	}
	
	FVector ForwardVector = CharacterOwner->GetActorForwardVector();
	FVector UpVector = CharacterOwner->GetActorUpVector();
	FVector LaunchVelocity = ForwardVector * GrenadeInitSpeed * FMath::Cos(FMath::DegreesToRadians(ThrowAngle)) +
		UpVector * GrenadeInitSpeed * FMath::Sin(FMath::DegreesToRadians(ThrowAngle));
	return LaunchVelocity;
}