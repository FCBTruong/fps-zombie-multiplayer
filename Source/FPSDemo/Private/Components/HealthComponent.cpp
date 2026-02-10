// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/HealthComponent.h"
#include "Net/UnrealNetwork.h"
#include <Characters/BaseCharacter.h>

// Sets default values for this component's properties
UHealthComponent::UHealthComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	MaxHealth = 100.0f;
	Health = MaxHealth;
	SetIsReplicatedByDefault(true);
}


// Called when the game starts
void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UHealthComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UHealthComponent::ApplyDamage(float DamageAmount)
{
	float CurHealth = Health;
	float NewHealth = CurHealth - DamageAmount;
	if (NewHealth < 0.0f)
	{
		NewHealth = 0.0f;
	}
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
	OnHealthUpdated.Broadcast(Health, MaxHealth);
}

void UHealthComponent::HealthDeath()
{
	UE_LOG(LogTemp, Warning, TEXT("Health has reached zero!"));
	OnDeath.Broadcast();
}

void UHealthComponent::SetHealth(float NewHealth)
{
	Health = FMath::Clamp(NewHealth, 0.0f, MaxHealth);
	if (Health == 0.0f)
	{
		HealthDeath();
	}

	OnRep_Health();
}

void UHealthComponent::SetMaxHealth(float NewMaxHealth)
{
	MaxHealth = FMath::Max(0.0f, NewMaxHealth);
	if (Health > MaxHealth)
	{
		SetHealth(Health);
		OnHealthUpdated.Broadcast(Health, MaxHealth);
	}
}

void UHealthComponent::ResetHealth()
{
	SetHealth(MaxHealth);
}