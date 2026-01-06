// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CharAudioComponent.generated.h"


class UAudioComponent;
class USoundBase;
class UCharacterAsset;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FPSDEMO_API UCharAudioComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCharAudioComponent();

public:
    void PlayFootstep();
    void PlayLanding();
    void PlayPlantSpike();
    void StopPlantSpike();
	void PlayDefuseSpike();
	void StopDefuseSpike();
	void PlaySound3D(USoundBase* Sound); // at actor location
	void PlayZombieSpawn();
	void PlayHeroDeath();
	void PlayHeroSpawn();
protected:
    virtual void BeginPlay() override;

private:
    // Sounds
    UPROPERTY(Transient)
    TObjectPtr<UCharacterAsset> CachedCharacterAsset;

    // Internal audio components
    UPROPERTY(Transient)
    TObjectPtr<UAudioComponent> FootstepComp;

    UPROPERTY(Transient)
    TObjectPtr<UAudioComponent> LoopingComp;

private:
    UAudioComponent* CreateAudioComp(FName Name);
    void PlayOneShot(USoundBase* Sound);
};
