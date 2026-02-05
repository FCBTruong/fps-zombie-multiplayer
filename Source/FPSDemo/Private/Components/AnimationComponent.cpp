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

void UAnimationComponent::PlayEquipMontage()
{
    if (!CachedCharacterAsset) {
        UE_LOG(LogTemp, Error, TEXT("PlayEquip: CachedCharacterAsset is null"));
        return;
	}
	if (CachedCharacterAsset->AnimMontage_Equip) {

		PlayMontage(CachedCharacterAsset->AnimMontage_Equip);
	}
}

void UAnimationComponent::PlayMontage(UAnimMontage* MontageToPlay)
{
    ABaseCharacter* OwnerChar = Cast<ABaseCharacter>(GetOwner());
    if (!OwnerChar)
    {
        UE_LOG(LogTemp, Error, TEXT("PlayMontage: Owner is not ABaseCharacter"));
        return;
    }

    if (!MontageToPlay)
    {
        UE_LOG(LogTemp, Error, TEXT("PlayMontage: MontageToPlay is null"));
        return;
    }

    auto PlayOnMesh = [&](USkeletalMeshComponent* MeshComp, const TCHAR* MeshLabel)
        {
            if (!MeshComp)
            {
                UE_LOG(LogTemp, Error, TEXT("PlayMontage: %s mesh is null"), MeshLabel);
                return;
            }

            UAnimInstance* AnimInst = MeshComp->GetAnimInstance();
            if (!AnimInst)
            {
                UE_LOG(
                    LogTemp,
                    Error,
                    TEXT("PlayMontage: AnimInstance is null on %s mesh (AnimClass=%s, Mode=%d)"),
                    MeshLabel,
                    *GetNameSafe(MeshComp->GetAnimClass()),
                    static_cast<int32>(MeshComp->GetAnimationMode())
                );
                return;
            }

            AnimInst->Montage_Play(MontageToPlay);
        };

    // Always play on TPS mesh (bots/others will use this).
    PlayOnMesh(OwnerChar->GetMesh(), TEXT("TPS"));

    // Only play on FPS mesh for locally controlled FPS view (avoid bot / remote issues).
    if (OwnerChar->IsFpsViewMode())
    {
        PlayOnMesh(OwnerChar->GetMeshFps(), TEXT("FPS"));
    }
}

void UAnimationComponent::PlayFireRifleMontage(FVector TargetPoint, UAnimMontage* FireMontage) {
    if (!CachedCharacterAsset) {
		return;
	}
    if (FireMontage) {
		UE_LOG(LogTemp, Warning, TEXT("Playing custom FireMontage"));
        PlayMontage(FireMontage);
	}
    else {
        PlayMontage(CachedCharacterAsset->AnimMontage_FireRifle); // default
    }
}

void UAnimationComponent::PlayFirePistolMontage(FVector TargetPoint) {
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
	UE_LOG(LogTemp, Warning, TEXT("Playing Zombie Attack Montage"));
    PlayMontage(CachedCharacterAsset->AnimMontage_ZombieAttack);
}