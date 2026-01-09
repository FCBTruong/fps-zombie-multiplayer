// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/RoleGatedComponent.h"
#include "Items/ItemIds.h"
#include "WeaponMeleeComponent.generated.h"

class ABaseCharacter;
class UActionStateComponent;
class UItemVisualComponent;
class UMeleeConfig;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FPSDEMO_API UWeaponMeleeComponent : public URoleGatedComponent
{
	GENERATED_BODY()

public:	
	UWeaponMeleeComponent();
	void RequestMeleeAttack(int32 AttackIndex);
	void HandleActiveItemChanged(EItemId MeleeId);
	void PlayHitFX_Local(
		const FVector& ImpactPoint,
		const FVector& ImpactNormal
	);

protected:
	virtual void BeginPlay() override;

private:
	void StartMelee_ServerAuth(int32 AttackIndex);
	void PerformMeleeTrace(int32 AttackIndex);

	UFUNCTION(Server, Reliable)
	void ServerStartMelee(int32 AttackIndex);

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlayMelee(int32 AttackIndex);
	bool CanMeleeNow() const;
	bool IsOwningClient() const;
	bool DoMeleeSweepMulti(
		TArray<FHitResult>& OutHits,
		float Range,
		float Radius
	) const;
	void PredictMeleeHitFX();
private:
	UPROPERTY(Transient) 
	TObjectPtr<const UMeleeConfig> MeleeConfig = nullptr;
	UPROPERTY(Transient) 
	TObjectPtr<UActionStateComponent> ActionStateComp = nullptr;
	UPROPERTY(Transient) TObjectPtr<UItemVisualComponent> VisualComp = nullptr;

	UPROPERTY(Transient) TObjectPtr<ABaseCharacter> Character = nullptr;
	FTimerHandle MeleeTraceTimer;

	FTimerHandle MeleeClientFxTimer;

	float MeleeRange = 100.f;
	float MeleeRadius = 50.f;
	float MeleeTraceDelay = 0.2f;
};
