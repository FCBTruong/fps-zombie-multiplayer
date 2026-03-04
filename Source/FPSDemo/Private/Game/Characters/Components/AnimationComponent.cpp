// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/Characters/Components/AnimationComponent.h"
#include "Game/Characters/BaseCharacter.h"
#include "Game/GameManager.h"

// Sets default values for this component's properties
UAnimationComponent::UAnimationComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UAnimationComponent::Init()
{
    OwnerCharacter = Cast<ABaseCharacter>(GetOwner());
    check(OwnerCharacter);
}

// Called when the game starts
void UAnimationComponent::BeginPlay()
{
	Super::BeginPlay();

    UGameManager* GameManager = UGameManager::Get(GetWorld());
    CachedCharacterAsset = GameManager->CharacterAsset.Get();
	check(CachedCharacterAsset);
}

void UAnimationComponent::PlayEquipMontage()
{
	if (CachedCharacterAsset && CachedCharacterAsset->AnimMontage_Equip) {

		PlayMontage(CachedCharacterAsset->AnimMontage_Equip);
	}
}

void UAnimationComponent::PlayMontage(UAnimMontage* MontageToPlay)
{
    if (!MontageToPlay)
    {
        return;
    }

    auto PlayOnMesh = [&](USkeletalMeshComponent* MeshComp, const TCHAR* MeshLabel)
        {
            if (!MeshComp)
            {
                return;
            }

            UAnimInstance* AnimInst = MeshComp->GetAnimInstance();
            if (!AnimInst)
            {
				UE_LOG(LogTemp, Warning, TEXT("AnimationComponent: No AnimInstance on %s mesh"), MeshLabel);    
                return;
            }

            AnimInst->Montage_Play(MontageToPlay);
        };

    // Always play on TPS mesh (bots/others will use this).
    PlayOnMesh(OwnerCharacter->GetMesh(), TEXT("TPS"));

    // Only play on FPS mesh for locally controlled FPS view (avoid bot / remote issues).
    if (OwnerCharacter->IsFpsViewMode())
    {
        PlayOnMesh(OwnerCharacter->GetMeshFps(), TEXT("FPS"));
    }
}

void UAnimationComponent::PlayFireRifleMontage(UAnimMontage* FireMontage) {
    if (!CachedCharacterAsset) {
        return;
	}

    if (FireMontage) {
        PlayMontage(FireMontage);
	}
    else {
        PlayMontage(CachedCharacterAsset->AnimMontage_FireRifle); // default
    }
}

void UAnimationComponent::PlayFirePistolMontage() {
    if (!CachedCharacterAsset) {
        return;
    }

    PlayMontage(CachedCharacterAsset->AnimMontage_FirePistol);
}

void UAnimationComponent::PlayReloadRifleMontage() {
    if (!CachedCharacterAsset) {
        return;
    }

    PlayMontage(CachedCharacterAsset->AnimMontage_ReloadRifle);
}

void UAnimationComponent::PlayReloadPistolMontage() {
    if (!CachedCharacterAsset) {
        return;
    }

    PlayMontage(CachedCharacterAsset->AnimMontage_ReloadPistol);
}

void UAnimationComponent::PlayThrowNadeMontage() {
    if (!CachedCharacterAsset) {
        return;
    }

    PlayMontage(CachedCharacterAsset->AnimMontage_ThrowNade);
}

void UAnimationComponent::PlayZombieAttackMontage() {
    if (!CachedCharacterAsset) {
        return;
    }

    PlayMontage(CachedCharacterAsset->AnimMontage_ZombieAttack);
}