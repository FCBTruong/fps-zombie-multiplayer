// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Items/ItemIds.h"
#include "WeaponMeleeComponent.generated.h"

class ABaseCharacter;
class UActionStateComponent;
class UItemVisualComponent;
class UMeleeConfig;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FPSDEMO_API UWeaponMeleeComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UWeaponMeleeComponent();
	void Initialize(
		UActionStateComponent* InAction,
		UItemVisualComponent* InVisual
	);
	void RequestMeleeAttack(int32 AttackIndex);
	void HandleActiveItemChanged(EItemId MeleeId);

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
private:
	UPROPERTY(Transient) 
	TObjectPtr<const UMeleeConfig> MeleeConfig = nullptr;
	UPROPERTY(Transient) 
	TObjectPtr<UActionStateComponent> ActionStateComp = nullptr;
	UPROPERTY(Transient) TObjectPtr<UItemVisualComponent> VisualComp = nullptr;

	UPROPERTY(Transient) TObjectPtr<ABaseCharacter> Character = nullptr;
	FTimerHandle MeleeTraceTimer;
};
