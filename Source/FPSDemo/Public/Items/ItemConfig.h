// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Items/ItemIds.h"
#include "Types/EquippedAnimState.h"
#include "ItemConfig.generated.h"

enum class EItemType : uint8 
{
	None,
	Firearm,
	Melee,
	Throwable,
	Armor
};

/**
 * 
 */
UCLASS()
class FPSDEMO_API UItemConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	EItemId Id;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	FText DisplayName;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	FText ItemDescription;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	TObjectPtr<UTexture2D> ItemIcon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	USkeletalMesh* Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
	UStaticMesh* StaticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	float ScaleOnHand = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	float ScaleOnHandTps = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	FName SocketNameFps = FName("ik_hand_gun");
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	FName SocketNameTps = FName("weapon_socket");

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	EEquippedAnimState AnimationState = EEquippedAnimState::Unarmed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	bool bIsDroppable = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	float Weight = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	int Price = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	FVector OffsetFps = FVector::ZeroVector;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Item")
	FVector OffsetTps = FVector::ZeroVector;

	virtual EItemType GetItemType() const { return EItemType::None; }
};
