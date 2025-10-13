// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Items/ItemData.h"
#include "Structs/InventoryItem.h"
#include "InventoryComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FPSDEMO_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	int32 IdCounter = 1000;

	UPROPERTY(Replicated)
	TArray<FInventoryItem> Items;
public:	
	// Sets default values for this component's properties
	UInventoryComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	int32 AddItem(const UItemData& ItemData);
	FInventoryItem* GetItemByInventoryId(int32 InventoryId);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	int32 GetItemCount() const { return Items.Num(); }
};
