// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/Characters/Components/CharAudioComponent.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundBase.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Game/Data/CharacterAsset.h"
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
    if (!GameManager)
    {
        UE_LOG(LogTemp, Error, TEXT("CharAudioComponent: GameManager is null"));
        return;
    }

    CachedCharacterAsset = GameManager->CharacterAsset.Get();
}

UAudioComponent* UCharAudioComponent::CreateAudioComp(FName Name)
{
    AActor* Owner = GetOwner();
    if (!Owner) return nullptr;

    USceneComponent* RootComp = Owner->GetRootComponent();
    if (!RootComp)
    {
        UE_LOG(LogTemp, Error, TEXT("CharAudioComponent: Owner root component is null"));
        return nullptr;
    }

    UAudioComponent* Comp = NewObject<UAudioComponent>(Owner, Name);
    if (!Comp)
    {
        return nullptr;
    }

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

    LoopingComp->SetSound(CachedCharacterAsset->Audio_PlantSpike);
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

void UCharAudioComponent::PlayZombieSpawn()
{
    if (!CachedCharacterAsset) {
        return;
    }
    PlaySound3D(CachedCharacterAsset->Audio_MonsterSpawn);
}

void UCharAudioComponent::PlayHeroDeath()
{
    if (!CachedCharacterAsset) {
        return;
    }
    PlaySound3D(CachedCharacterAsset->Audio_HeroDead);
}

void UCharAudioComponent::PlayZombieDeath()
{
    if (!CachedCharacterAsset) {
        return;
    }
    PlaySound3D(CachedCharacterAsset->Audio_SoldierDead);
}

void UCharAudioComponent::PlaySoldierDeath()
{
    if (!CachedCharacterAsset) {
        return;
    }
    PlaySound3D(CachedCharacterAsset->Audio_SoldierDead);
}

void UCharAudioComponent::PlayHeroSpawn()
{
    if (!CachedCharacterAsset) {
        return;
    }
    PlaySound3D(CachedCharacterAsset->Audio_HeroSpawn);
}

void UCharAudioComponent::PlayHeal()
{
    if (!CachedCharacterAsset) {
        return;
    }
    PlaySound3D(CachedCharacterAsset->Audio_Heal);
}