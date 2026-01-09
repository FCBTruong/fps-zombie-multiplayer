// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/RoleGatedComponent.h"
#include "ItemUseComponent.generated.h"

class ABaseCharacter;
class UEquipComponent;
class UActionStateComponent;
class UWeaponFireComponent;
class UWeaponMeleeComponent;
class UThrowableComponent;
class USpikeComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FPSDEMO_API UItemUseComponent : public URoleGatedComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UItemUseComponent();
	virtual void BeginPlay() override;

	void PrimaryPressed();
	void PrimaryReleased();
	void SecondaryPressed();
	void SecondaryReleased();
	void ReloadPressed();
private:

	UPROPERTY()
	TObjectPtr<ABaseCharacter> OwnerChar = nullptr;
	UPROPERTY()
	TObjectPtr<UEquipComponent> EquipComp = nullptr;
	UPROPERTY()
	TObjectPtr<UActionStateComponent> ActionStateComp = nullptr;
	UPROPERTY()
	TObjectPtr<UWeaponFireComponent> WeaponFireComp = nullptr;
	UPROPERTY()
	TObjectPtr<UWeaponMeleeComponent> WeaponMeleeComp = nullptr;
	UPROPERTY()
	TObjectPtr<UThrowableComponent> ThrowableComp = nullptr;
	UPROPERTY()
	TObjectPtr<USpikeComponent> SpikeComp = nullptr;
};
