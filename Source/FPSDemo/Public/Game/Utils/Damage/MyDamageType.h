// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/DamageType.h"
#include "Shared/Types/ItemId.h"
#include "MyDamageType.generated.h"

/**
 * 
 */
UCLASS()
class FPSDEMO_API UMyDamageType : public UDamageType
{
	GENERATED_BODY()
public:
	UPROPERTY()
	EItemId WeaponId;
};
