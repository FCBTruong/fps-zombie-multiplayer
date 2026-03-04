// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Game/Characters/Components/RoleGatedComponent.h"
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
	void Init();

	void PrimaryPressed();
	void PrimaryReleased();
	void SecondaryPressed();
	void SecondaryReleased();
	void ReloadPressed();
private:
	ABaseCharacter* OwnerChar = nullptr;
	UEquipComponent* EquipComp = nullptr;
	UActionStateComponent* ActionStateComp = nullptr;
	UWeaponFireComponent* WeaponFireComp = nullptr;
	UWeaponMeleeComponent* WeaponMeleeComp = nullptr;
	UThrowableComponent* ThrowableComp = nullptr;
	USpikeComponent* SpikeComp = nullptr;
};
