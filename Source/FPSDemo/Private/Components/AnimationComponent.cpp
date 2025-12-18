// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/AnimationComponent.h"
#include "Characters/BaseCharacter.h"

// Sets default values for this component's properties
UAnimationComponent::UAnimationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UAnimationComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UAnimationComponent::PlayEquip(EWeaponTypes WeaponType)
{
	ABaseCharacter* Owner = Cast<ABaseCharacter>(GetOwner());
	if (Montages.Equip && Owner) {

		UE_LOG(LogTemp, Warning, TEXT("Playing Equip Montage for Rifle"));
		Owner->GetCurrentMesh()->GetAnimInstance()->Montage_Play(Montages.Equip);
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
	PlayMontage(Montages.FireRifle);
}

void UAnimationComponent::PlayFirePistolMontage(FVector TargetPoint) {
    PlayMontage(Montages.FirePistol);
}

void UAnimationComponent::PlayMeleeAttackAnimation(int32 AttackIndex) {
    if (AttackIndex == 0) {
        PlayMontage(Montages.KnifeAttack1);
    }
    else if (AttackIndex == 1) {
        PlayMontage(Montages.KnifeAttack2);
    }
}

void UAnimationComponent::PlayReloadMontage() {
    PlayMontage(Montages.ReloadRifle);
}

void UAnimationComponent::PlayThrowNadeMontage() {
    PlayMontage(Montages.ThrowNade);
}

void UAnimationComponent::PlayHoldNadeMontage() {
    PlayMontage(Montages.HoldNade);
}