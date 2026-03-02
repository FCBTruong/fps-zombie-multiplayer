// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Spike.generated.h"

class USpringArmComponent;
class USpikeComponent;
class ABaseCharacter;

UCLASS()
class FPSDEMO_API ASpike : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASpike();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	bool bIsDefused = false;
	bool bIsExploding = false;
	bool bIsDefuseInProgress = false;
	float ExplodeTimer = 0.f;
	float CamYaw = 0.f;
	float CamPitch = -20.f;
	FTimerHandle TimerHandle_Explode;
	FTimerHandle DefuseTimerHandle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Explode")
	UStaticMeshComponent* ExplodeSphere;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Explode")
	UStaticMeshComponent* MainMeshRef;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Explode")
	USoundBase* ActiveSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Explode")
	USoundBase* ExplodeSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Explode")
	USoundBase* SpikePlantedSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Explode")
	USoundBase* SpikeDefuseedVoice;

	UPROPERTY()
	USpringArmComponent* SpringArmComp;

	UPROPERTY()
	UAudioComponent* ActiveSoundComp;

	UPROPERTY(EditAnywhere, Category = "Camera")
	float LookSensitivity = 2.0f;

	UPROPERTY(EditAnywhere, Category = "Camera")
	float MinPitch = -70.f;

	UPROPERTY(EditAnywhere, Category = "Camera")
	float MaxPitch = 80.f;
public:	
	static constexpr float TimeExplode = 26.0f;
	static constexpr float DefuseTime = 6.f;
	static constexpr float ExplodeDamage = 500.f;

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_Defused();

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_Explode();

	bool IsDefuseInProgress() const;
	bool IsDefused() const;
	void Defused();
	void StartDefuse(USpikeComponent* DefuseComp);
	void CancelDefuse();
	void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	void AddCameraYaw(float DeltaYaw);
	void AddCameraPitch(float DeltaPitch);
private:
	TWeakObjectPtr<USpikeComponent> DefusingComponent;

	void OnCompleteExplode();
	void Explode();
	void OnCharacterDead(ABaseCharacter* DeadCharacter);
};
