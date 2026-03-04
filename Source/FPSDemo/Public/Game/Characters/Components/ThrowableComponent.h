// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Game/Characters/Components/RoleGatedComponent.h"
#include "Shared/Types/ItemId.h"
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
	UThrowableComponent();
	void Init();

	void RequestStartThrow();
	void OnNadeRelease();

protected:
	virtual void OnEnabledChanged(bool bNowEnabled) override;

private:
	float GrenadeInitSpeed = 1200.f;
	float ThrowAngle = 0.f;

	ABaseCharacter* CharacterOwner;
	UAnimationComponent* AnimComp;
	UEquipComponent* EquipComp;
	UActionStateComponent* ActionStateComp;

	FTimerHandle TimerHandle_FinishThrow;
private:
	void HandleThrow();
	void FinishThrow();
	bool CanStartThrow() const;

	UFUNCTION(Server, Reliable)
	void ServerThrow();

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastThrowAction();

	FVector ComputeThrowVelocity() const;
};
