// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Shared/Types/ItemId.h"
#include "Shared/Data/Items/ItemConfig.h"
#include "ItemVisualComponent.generated.h"

class UEquipComponent;
class UGameManager;
class ABaseCharacter;
class AEquippedItem;
class UCharCameraComponent;
class UAnimationComponent;
class ABaseCharacter;


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FPSDEMO_API UItemVisualComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UItemVisualComponent();

    void OnNotifyGrabMag();
    void OnNotifyInsertMag();
    void HideItemVisual();
	void OnOwnerDead();
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
    void PlayFireFX(FVector TargetPoint);
    void PlayMeleeAttack(UAnimMontage* Anim);

private: 
    UEquipComponent* EquipComp = nullptr;
    UCharCameraComponent* CameraComp = nullptr;
    UAnimationComponent* AnimComp = nullptr;
	ABaseCharacter* Character = nullptr;

    // Cosmetic weapon actor (NOT replicated)
    UPROPERTY(Transient) 
    TObjectPtr<AEquippedItem> EquippedActor = nullptr;

    // Delegate handler
    UFUNCTION()
    void HandleActiveItemChanged(EItemId NewItemId);
    void RefreshVisual(EItemId NewItemId);
    void DestroyVisual();
    void AttachToHands(const UItemConfig* Data);
	const UItemConfig* GetItemConfig(EItemId ItemId) const;
    void OnViewModeChanged(bool bIsFPS);
};
