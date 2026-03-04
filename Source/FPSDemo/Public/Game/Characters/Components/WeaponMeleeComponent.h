// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Game/Characters/Components/RoleGatedComponent.h"
#include "Shared/Types/ItemId.h"
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
	void Init();

	void RequestMeleeAttack(int32 AttackIndex);
	void HandleActiveItemChanged(EItemId MeleeId);
	void PlayHitFX_Local(
		const FVector& ImpactPoint,
		const FVector& ImpactNormal
	);

private:
	UFUNCTION(Server, Reliable)
	void ServerStartMelee(int32 AttackIndex);

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlayMelee(int32 AttackIndex);

	void StartMelee_ServerAuth(int32 AttackIndex);
	void PerformMeleeTrace(int32 AttackIndex);
	void PredictMeleeHitFX();

	bool CanMeleeNow() const;
	bool IsOwningClient() const;
	bool DoMeleeSweepMulti(
		TArray<FHitResult>& OutHits,
		float Range,
		float Radius
	) const;
private:
	UPROPERTY(Transient) 
	TObjectPtr<const UMeleeConfig> MeleeConfig = nullptr;

	UActionStateComponent* ActionStateComp = nullptr;
	UItemVisualComponent* VisualComp = nullptr;
	ABaseCharacter* Character = nullptr;

	FTimerHandle MeleeTraceTimer;
	FTimerHandle MeleeClientFxTimer;

	float MeleeRange = 100.f;
	float MeleeRadius = 50.f;
	float MeleeTraceDelay = 0.2f;
	float LastAttackTime = -FLT_MAX;
};
