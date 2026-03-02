// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/Characters/Components/HealthComponent.h"
#include "Net/UnrealNetwork.h"
#include "Game/Characters/BaseCharacter.h"

// Sets default values for this component's properties
UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	MaxHealth = 100.0f;
	Health = MaxHealth;
	SetIsReplicatedByDefault(true);
}

void UHealthComponent::ApplyDamage(float DamageAmount)
{
	if (DamageAmount <= 0.f)
	{
		UE_LOG(LogTemp, Warning, TEXT("ApplyDamage called with non-positive damage amount: %f"), DamageAmount);
		return;
	}

	const float NewHealth = FMath::Max(0.0f, Health - DamageAmount);
	SetHealth(NewHealth);
}

void UHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UHealthComponent, Health);
	DOREPLIFETIME(UHealthComponent, MaxHealth);
}

void UHealthComponent::OnRep_Health()
{
	BroadcastHealthUpdated();
}

void UHealthComponent::BroadcastHealthUpdated()
{
	OnHealthUpdated.Broadcast(Health, MaxHealth);
}

void UHealthComponent::HandleDeath()
{
	OnDeath.Broadcast();
}

void UHealthComponent::SetHealth(float NewHealth)
{
	AActor* Owner = GetOwner();
	if (!Owner->HasAuthority())
	{
		return;
	}

	const float ClampedHealth = FMath::Clamp(NewHealth, 0.0f, MaxHealth);
	if (FMath::IsNearlyEqual(Health, ClampedHealth))
	{
		return;
	}

	const float OldHealth = Health;
	Health = ClampedHealth;

	if (OldHealth > 0.0f && Health == 0.0f)
	{
		HandleDeath();
	}

	BroadcastHealthUpdated();
}

void UHealthComponent::SetMaxHealth(float NewMaxHealth)
{
	AActor* Owner = GetOwner();
	if (!Owner->HasAuthority())
	{
		return;
	}

	const float OldMaxHealth = MaxHealth;
	const float ClampedMaxHealth = FMath::Max(0.0f, NewMaxHealth);

	if (FMath::IsNearlyEqual(OldMaxHealth, ClampedMaxHealth))
	{
		return;
	}

	MaxHealth = ClampedMaxHealth;

	if (Health > MaxHealth)
	{
		SetHealth(Health);
		return;
	}

	BroadcastHealthUpdated();
}

void UHealthComponent::ResetHealth()
{
	SetHealth(MaxHealth);
}

bool UHealthComponent::ApplyHeal(float Amount)
{
	if (Amount <= 0.f) {
		return false;
	}
	if (IsAtMaxHealth()) {
		return false;
	}

	float NewHealth = FMath::Clamp(Health + Amount, 0.f, MaxHealth);

	if (NewHealth == Health) {
		return false;
	}

	SetHealth(NewHealth);
	
	return true;
}

float UHealthComponent::GetHealthPercent() const
{
	if (MaxHealth <= 0.f) {
		return 0.f;
	}
	return Health / MaxHealth;
}