// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "CharacterAsset.generated.h"

class UNiagaraSystem;
/**
 * 
 */
UCLASS()
class FPSDEMO_API UCharacterAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> AnimMontage_Equip = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> AnimMontage_ThrowNade = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> AnimMontage_FireRifle = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> AnimMontage_FirePistol = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> AnimMontage_ReloadRifle = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> AnimMontage_ReloadPistol = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> AnimMontage_ZombieAttack = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	TObjectPtr<UAnimMontage> AnimMontage_HitReact = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Flash")
	TObjectPtr<UCurveFloat> StunCurve = nullptr;


	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	TObjectPtr<USoundBase> Audio_Footstep = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	TObjectPtr<USoundBase> Audio_Landing = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	TObjectPtr<USoundBase> Audio_PlantSpike = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	TObjectPtr<USoundBase> Audio_DefuseSpike = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	TObjectPtr<USoundBase> Audio_MonsterSpawn = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	TObjectPtr<USoundBase> Audio_HeroDead = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	TObjectPtr<USoundBase> Audio_SoldierDead = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	TObjectPtr<USoundBase> Audio_ZombieDead = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Audio")
	TObjectPtr<USoundBase> Audio_HeroSpawn = nullptr;

	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	TSubclassOf<AActor> DeathCameraProxyClass;

	UPROPERTY(EditDefaultsOnly, Category = "FX")
	TObjectPtr<UNiagaraSystem> BloodFx;

	UPROPERTY(EditDefaultsOnly, Category = "FX")
	TObjectPtr<UParticleSystem> HitFx;

	UPROPERTY(EditDefaultsOnly, Category = "FX")
	TObjectPtr<UNiagaraSystem> TurnToZombieFx;

	UPROPERTY(EditDefaultsOnly, Category = "FX")
	TObjectPtr<UParticleSystem> HeroFx;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mesh")
	USkeletalMesh* TerroristMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mesh")
	USkeletalMesh* FpsMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mesh")
	USkeletalMesh* CounterTerroristMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mesh")
	USkeletalMesh* ZombieMeshTPS;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mesh")
	USkeletalMesh* ZombieMeshFPS;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mesh")
	USkeletalMesh* HeroMeshFPS;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mesh")
	USkeletalMesh* HeroMeshTPS;

	// anim
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mesh")
	TSubclassOf<UAnimInstance> ZombieAnimTPS;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mesh")
	TSubclassOf<UAnimInstance> ZombieAnimFPS;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mesh")
	TSubclassOf<UAnimInstance> HumanAnimFPS;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mesh")
	TSubclassOf<UAnimInstance> HumanAnimTPS;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mesh")
	TSubclassOf<UAnimInstance> HeroAnimFPS;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mesh")
	TSubclassOf<UAnimInstance> HeroAnimTPS;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UMaterialInterface* FlashPPMat;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UCurveFloat* CrouchCurve;
};

