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
	Health -= DamageAmount;
	if (Health < 0.0f)
	{
		Health = 0.0f;
	}
	if (Health == 0.0f)
	{
		HealthDeath();
	}
}




void UHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UHealthComponent, Health);
}

void UHealthComponent::OnRep_Health()
{
	UE_LOG(LogTemp, Log, TEXT("Health replicated: %f"), Health);
	OnHealthUpdated.Broadcast();
}

void UHealthComponent::HealthDeath()
{
	UE_LOG(LogTemp, Warning, TEXT("Health has reached zero!"));
	// Implement death logic here (e.g., notify the owning actor, play death animation, etc.)
	ABaseCharacter* OwnerCharacter = Cast<ABaseCharacter>(GetOwner());
	if (!OwnerCharacter) return;
	
	// Optional: broadcast death event
	OnDeath.Broadcast();
}