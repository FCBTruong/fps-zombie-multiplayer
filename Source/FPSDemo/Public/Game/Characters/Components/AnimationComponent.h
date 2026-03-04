// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Game/Data/CharacterAsset.h"
#include "AnimationComponent.generated.h"

class ABaseCharacter;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FPSDEMO_API UAnimationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UAnimationComponent();
	void Init();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(Transient)
	TObjectPtr<UCharacterAsset> CachedCharacterAsset;

	ABaseCharacter* OwnerCharacter;

public:	
	void PlayEquipMontage();
	void PlayFireRifleMontage(UAnimMontage* FireMontage);
	void PlayFirePistolMontage();
	void PlayMontage(UAnimMontage* MontageToPlay);
	void PlayReloadRifleMontage();
	void PlayReloadPistolMontage();
	void PlayThrowNadeMontage();
	void PlayZombieAttackMontage();
};
