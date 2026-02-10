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
	
	bool bIsDefused = false;

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

	// explosion animation state
	bool bIsExploding = false;
	float ExplodeTimer = 0.f;
	FTimerHandle TimerHandle_Explode;
	UPROPERTY()
	UAudioComponent* ActiveSoundComp;
	bool bIsDefuseInProgress = false;

	float CamYaw = 0.f;
	float CamPitch = -20.f;

	UPROPERTY(EditAnywhere, Category = "Camera")
	float LookSensitivity = 2.0f;

	UPROPERTY(EditAnywhere, Category = "Camera")
	float MinPitch = -80.f;

	UPROPERTY(EditAnywhere, Category = "Camera")
	float MaxPitch = 20.f;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	void Explode();
	static constexpr float TimeExplode = 26.0f;
	void Defused();

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_Defused();

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_Explode();

	void StartDefuse(USpikeComponent* DefuseComp);

	FTimerHandle DefuseTimerHandle;

	USpikeComponent* DefusingComponent;

	bool IsDefuseInProgress() const;

	void CancelDefuse();

	bool IsDefused() const { return bIsDefused; }
	void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	void AddCameraYaw(float DeltaYaw);

private:
	void OnCompleteExplode();
	void OnCharacterDead(ABaseCharacter* DeadCharacter);
};
