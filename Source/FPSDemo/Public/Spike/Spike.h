// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Spike.generated.h"

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
	
	bool IsDefused = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Explode")
	UStaticMeshComponent* ExplodeSphere;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Explode")
	USoundBase* ActiveSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Explode")
	USoundBase* ExplodeSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Explode")
	USoundBase* SpikePlantedSound;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Explode")
	USoundBase* SpikeDefuseedVoice;

	// explosion animation state
	bool bIsExploding = false;
	float ExplodeTimer = 0.f;
	FTimerHandle TimerHandle_Explode;
	UPROPERTY()
	UAudioComponent* ActiveSoundComp;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	void Explode();
	static constexpr float TimeExplode = 36.0f;
	void Defused();

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_Defused();

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_Explode();
};
