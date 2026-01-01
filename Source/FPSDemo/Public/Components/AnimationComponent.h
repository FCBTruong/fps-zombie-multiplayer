// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Asset/CharacterAsset.h"
#include "AnimationComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FPSDEMO_API UAnimationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UAnimationComponent();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(Transient)
	TObjectPtr<UCharacterAsset> CachedCharacterAsset;

public:	
	void PlayEquipMontage();
	void PlayFireRifleMontage(FVector TargetPoint);
	void PlayFirePistolMontage(FVector TargetPoint);
	void PlayMeleeAttackMontage(int32 AttackIndex);
	void PlayMontage(UAnimMontage* MontageToPlay);
	void PlayReloadRifleMontage();
	void PlayReloadPistolMontage();
	void PlayThrowNadeMontage();
};
