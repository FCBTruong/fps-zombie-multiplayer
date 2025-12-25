// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/CharAudioComponent.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Asset/CharacterAsset.h"
#include "Game/GameManager.h"

UCharAudioComponent::UCharAudioComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UCharAudioComponent::BeginPlay()
{
    Super::BeginPlay();

    FootstepComp = CreateAudioComp(TEXT("FootstepAudioComp"));
    LoopingComp = CreateAudioComp(TEXT("LoopingAudioComp"));
    UGameManager* GameManager = UGameManager::Get(GetWorld());
    CachedCharacterAsset = GameManager->CharacterAsset.Get();
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
    if (!CachedCharacterAsset) {
		return;
    }
    PlayOneShot(CachedCharacterAsset->Audio_Footstep);
}

void UCharAudioComponent::PlayLanding()
{
    if (!CachedCharacterAsset) {
		return;
    }
    PlayOneShot(CachedCharacterAsset->Audio_Landing);
}

void UCharAudioComponent::PlayPlantSpike()
{
    if (!CachedCharacterAsset) {
        return;
    }
    if (!CachedCharacterAsset->Audio_PlantSpike || !LoopingComp) return;

    LoopingComp->SetSound(CachedCharacterAsset->Audio_PlantSpike);
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
    if (!CachedCharacterAsset || !LoopingComp) return;

    LoopingComp->SetSound(CachedCharacterAsset->Audio_DefuseSpike);
    LoopingComp->Play();
}

void UCharAudioComponent::StopDefuseSpike()
{
    if (LoopingComp)
    {
        LoopingComp->Stop();
    }
}

void UCharAudioComponent::PlaySound3D(USoundBase* Sound)
{
    if (!Sound) return;
	AActor* Owner = GetOwner();
	if (!Owner) return;
    UGameplayStatics::PlaySoundAtLocation(
        this,
        Sound,
		Owner->GetActorLocation()
    );
}