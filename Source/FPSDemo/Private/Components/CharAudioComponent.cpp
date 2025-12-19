// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/CharAudioComponent.h"

#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "GameFramework/Actor.h"

UCharAudioComponent::UCharAudioComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UCharAudioComponent::BeginPlay()
{
    Super::BeginPlay();

    FootstepComp = CreateAudioComp(TEXT("FootstepAudioComp"));
    LoopingComp = CreateAudioComp(TEXT("LoopingAudioComp"));
}

UAudioComponent* UCharAudioComponent::CreateAudioComp(FName Name)
{
    AActor* Owner = GetOwner();
    if (!Owner) return nullptr;

    UAudioComponent* Comp = NewObject<UAudioComponent>(Owner, Name);
    Comp->bAutoActivate = false;
    Comp->RegisterComponent();
    Comp->AttachToComponent(
        Owner->GetRootComponent(),
        FAttachmentTransformRules::KeepRelativeTransform
    );

    return Comp;
}

void UCharAudioComponent::PlayOneShot(USoundBase* Sound)
{
    if (!Sound || !FootstepComp) return;

    FootstepComp->SetSound(Sound);
    FootstepComp->Play();
}

// ===== Public API =====

void UCharAudioComponent::PlayFootstep()
{
    PlayOneShot(Sounds.Footstep);
}

void UCharAudioComponent::PlayLanding()
{
    PlayOneShot(Sounds.Landing);
}

void UCharAudioComponent::PlayPlantSpike()
{
    if (!Sounds.PlantSpike || !LoopingComp) return;

    LoopingComp->SetSound(Sounds.PlantSpike);
    LoopingComp->Play();
}

void UCharAudioComponent::StopPlantSpike()
{
    if (LoopingComp)
    {
        LoopingComp->Stop();
    }
}

void UCharAudioComponent::PlayDefuseSpike()
{
    if (!Sounds.DefuseSpike || !LoopingComp) return;

    LoopingComp->SetSound(Sounds.DefuseSpike);
    LoopingComp->Play();
}

void UCharAudioComponent::StopDefuseSpike()
{
    if (LoopingComp)
    {
        LoopingComp->Stop();
    }
}