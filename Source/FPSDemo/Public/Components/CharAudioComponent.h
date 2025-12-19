// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CharAudioComponent.generated.h"


class UAudioComponent;
class USoundBase;

USTRUCT(BlueprintType)
struct FCharacterSoundSet
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, Category = "Audio")
    TObjectPtr<USoundBase> Footstep = nullptr;

    UPROPERTY(EditDefaultsOnly, Category = "Audio")
    TObjectPtr<USoundBase> Landing = nullptr;

    UPROPERTY(EditDefaultsOnly, Category = "Audio")
    TObjectPtr<USoundBase> PlantSpike = nullptr;

    UPROPERTY(EditDefaultsOnly, Category = "Audio")
    TObjectPtr<USoundBase> DefuseSpike = nullptr;
};

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

protected:
    virtual void BeginPlay() override;

private:
    // Sounds
    UPROPERTY(EditDefaultsOnly, Category = "Audio")
    FCharacterSoundSet Sounds;

    // Internal audio components
    UPROPERTY(Transient)
    TObjectPtr<UAudioComponent> FootstepComp;

    UPROPERTY(Transient)
    TObjectPtr<UAudioComponent> LoopingComp;

private:
    UAudioComponent* CreateAudioComp(FName Name);
    void PlayOneShot(USoundBase* Sound);
};
