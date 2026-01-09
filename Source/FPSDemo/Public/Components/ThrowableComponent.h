// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/RoleGatedComponent.h"
#include "Items/ItemIds.h"
#include "ThrowableComponent.generated.h"

class ABaseCharacter;
class UAnimationComponent;
class UEquipComponent;
class UActionStateComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FPSDEMO_API UThrowableComponent : public URoleGatedComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UThrowableComponent();

	void RequestStartThrow();
	void OnNadeRelease();
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void OnEnabledChanged(bool bNowEnabled) override;

private:
	float GrenadeInitSpeed = 1200.f;
	float ThrowAngle = 0.f;

	UPROPERTY()
	TObjectPtr<ABaseCharacter> CharacterOwner;
	UPROPERTY()
	TObjectPtr<UAnimationComponent> AnimComp;
	UPROPERTY()
	TObjectPtr<UEquipComponent> EquipComp;
	UPROPERTY()
	TObjectPtr<UActionStateComponent> ActionStateComp;

	FTimerHandle TimerHandle_FinishThrow;
private:
	void HandleThrow();

	UFUNCTION(Server, Reliable)
	void ServerThrow();

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastThrowAction();

	bool CanStartThrow() const;
	void FinishThrow();

	FVector ComputeThrowVelocity() const;
};
