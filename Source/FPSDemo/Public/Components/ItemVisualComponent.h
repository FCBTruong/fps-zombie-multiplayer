// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Items/ItemIds.h"
#include "Items/ItemConfig.h"
#include "ItemVisualComponent.generated.h"

class UEquipComponent;
class UGameManager;
class ABaseCharacter;
class AEquippedItem;
class UCharCameraComponent;
class UAnimationComponent;


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
protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	void OnViewModeChanged(bool bIsFPS);
    void Initialize(UEquipComponent* InEquipComp, UCharCameraComponent* InCameraComp, UAnimationComponent* InAnimComp);
    void PlayFireFX(FVector TargetPoint);
	void PlayMeleeAttack(int32 AttackIndex);

private:
    UPROPERTY(Transient) 
    TObjectPtr<UEquipComponent> EquipComp = nullptr;
    UPROPERTY(Transient)
    TObjectPtr<UCharCameraComponent> CameraComp;
    UPROPERTY(Transient)
	TObjectPtr<UAnimationComponent> AnimComp = nullptr;
    UPROPERTY(Transient) 
    TObjectPtr<UGameManager> CachedGM = nullptr;

    // Cosmetic weapon actor (NOT replicated)
    UPROPERTY(Transient) TObjectPtr<AEquippedItem> EquippedActor = nullptr;

    // Delegate handler
    UFUNCTION()
    void HandleActiveItemChanged(EItemId NewItemId);

    // Core
    void RefreshVisual(EItemId NewItemId);
    void DestroyVisual();

    ABaseCharacter* GetCharacter() const;

    void AttachToHands(const UItemConfig* Data);
    bool ShouldSpawnVisuals() const;
	const UItemConfig* GetItemConfig(EItemId ItemId) const;
};
