// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"


DECLARE_MULTICAST_DELEGATE(FOnDeath);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnHealthUpdated, float, float);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))

class FPSDEMO_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	UPROPERTY(ReplicatedUsing = OnRep_Health)
	float Health;
	UPROPERTY(Replicated)
	float MaxHealth;

	UFUNCTION()
	void OnRep_Health();
public:	
	// Sets default values for this component's properties
	UHealthComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	UFUNCTION(BlueprintCallable)
	float GetHealthPercent() const { return Health / MaxHealth; }
	float GetHealth() const { return Health; }
	float GetMaxHealth() const { return MaxHealth; }

	void ApplyDamage(float DamageAmount);
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void HealthDeath();
	void ResetHealth();
	void SetHealth(float NewHealth);
	void SetMaxHealth(float NewMaxHealth);
	bool IsAlive() const { return Health > 0.f; }
	bool IsDead() const { return Health <= 0.f; }
	bool IsAtMaxHealth() const { return Health >= MaxHealth; }
	bool ApplyHeal(float Amount);

	// Delegates
	FOnHealthUpdated OnHealthUpdated;
	FOnDeath OnDeath;
};
