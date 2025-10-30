// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"


DECLARE_MULTICAST_DELEGATE(FOnDeath);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHealthUpdated);
UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))

class FPSDEMO_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	UPROPERTY(ReplicatedUsing = OnRep_Health)
	float Health;
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
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void ApplyDamage(float DamageAmount);
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintCallable)
	float GetHealthPercent() const { return Health / MaxHealth; }

	UPROPERTY()
	FOnHealthUpdated OnHealthUpdated;

	float GetHealth() const { return Health; }
	float GetMaxHealth() const { return MaxHealth; }
	void HealthDeath();
	FOnDeath OnDeath;
};
