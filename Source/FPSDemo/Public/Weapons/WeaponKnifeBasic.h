// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponTypes.h"
#include "WeaponBase.h"
#include "WeaponKnifeBasic.generated.h"

/**
 * 
 */
UCLASS()
class FPSDEMO_API AWeaponKnifeBasic : public AWeaponBase
{
	GENERATED_BODY()

public:
	EWeaponTypes GetWeaponType() override;
};
