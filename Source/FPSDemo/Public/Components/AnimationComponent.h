// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Weapons/WeaponTypes.h"
#include "AnimationComponent.generated.h"

USTRUCT(BlueprintType)
struct FCharacterMontageSet
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> Equip = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> ThrowNade = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> HoldNade = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> KnifeAttack1 = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> KnifeAttack2 = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> FireRifle = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> FirePistol = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> ReloadRifle = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> ReloadPistol = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> FireMelee = nullptr;
};



UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FPSDEMO_API UAnimationComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UAnimationComponent();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	FCharacterMontageSet Montages;

public:	
	void PlayEquip(EWeaponTypes WeaponType);
	void PlayFireRifleMontage(FVector TargetPoint);
	void PlayFirePistolMontage(FVector TargetPoint);
	void PlayMeleeAttackAnimation(int32 AttackIndex);
	void PlayMontage(UAnimMontage* MontageToPlay);
	void PlayReloadMontage();
	void PlayThrowNadeMontage();
	void PlayHoldNadeMontage();
};
