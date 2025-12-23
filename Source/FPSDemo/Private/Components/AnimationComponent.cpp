// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/AnimationComponent.h"
#include "Characters/BaseCharacter.h"
#include "Game/GameManager.h"

// Sets default values for this component's properties
UAnimationComponent::UAnimationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

    // load animations

}


// Called when the game starts
void UAnimationComponent::BeginPlay()
{
	Super::BeginPlay();

    UGameManager* GameManager = UGameManager::Get(GetWorld());
	CachedCharacterAsset = GameManager->CharacterAsset.Get();
}

void UAnimationComponent::PlayEquip(EWeaponTypes WeaponType)
{
	ABaseCharacter* Owner = Cast<ABaseCharacter>(GetOwner());
    if (!Owner) {
        UE_LOG(LogTemp, Error, TEXT("PlayEquip: Owner is null"));
        return;
	}
    if (!CachedCharacterAsset) {
        UE_LOG(LogTemp, Error, TEXT("PlayEquip: CachedCharacterAsset is null"));
        return;
	}
	if (CachedCharacterAsset->AnimMontage_Equip) {

		UE_LOG(LogTemp, Warning, TEXT("Playing Equip Montage for Rifle"));
		Owner->GetCurrentMesh()->GetAnimInstance()->Montage_Play(CachedCharacterAsset->AnimMontage_Equip);
	}
}

void UAnimationComponent::PlayMontage(UAnimMontage* MontageToPlay)
{
	ABaseCharacter* Owner = Cast<ABaseCharacter>(GetOwner());
	if (MontageToPlay && Owner) {
        if (!MontageToPlay)
        {
            UE_LOG(LogTemp, Error, TEXT("MontageToPlay is null"));
            return;
        }
        USkeletalMeshComponent* MeshComp = Owner->GetCurrentMesh();
        if (!MeshComp)
        {
            UE_LOG(LogTemp, Error, TEXT("GetCurrentMesh() returned null"));
            return;
        }
        UAnimInstance* AnimInst = MeshComp->GetAnimInstance();
        if (!AnimInst)
        {
            UE_LOG(LogTemp, Error, TEXT("AnimInstance is null"));
            return;
        }
        AnimInst->Montage_Play(MontageToPlay);
	}
}

void UAnimationComponent::PlayFireRifleMontage(FVector TargetPoint) {
    if (!CachedCharacterAsset) {
		return;
	}
	PlayMontage(CachedCharacterAsset->AnimMontage_FireRifle);
}

void UAnimationComponent::PlayFirePistolMontage(FVector TargetPoint) {
    if (!CachedCharacterAsset) {
        return;
    }
    PlayMontage(CachedCharacterAsset->AnimMontage_FirePistol);
}

void UAnimationComponent::PlayMeleeAttackAnimation(int32 AttackIndex) {
    if (!CachedCharacterAsset) {
        return;
	}
    if (AttackIndex == 0) {
        PlayMontage(CachedCharacterAsset->AnimMontage_KnifeAttack1);
    }
    else if (AttackIndex == 1) {
        PlayMontage(CachedCharacterAsset->AnimMontage_KnifeAttack2);
    }
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

void UAnimationComponent::PlayHoldNadeMontage() {
    if (!CachedCharacterAsset) {
        return;
    }
    PlayMontage(CachedCharacterAsset->AnimMontage_HoldNade);
}