// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Items/ItemData.h"
#include "Structs/InventoryItem.h"
#include "Weapons/WeaponTypes.h"
#include "Game/GameManager.h"
#include "InventoryComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FPSDEMO_API UInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

private:
	int32 IdCounter = 1000;

	//UPROPERTY(ReplicatedUsing = OnRep_Items)
	UPROPERTY(ReplicatedUsing = OnRep_Items)
	TArray<FInventoryItem> Items;

	UGameManager* GMR;
	TMap<int32, int32> SlotMap; // Slot -> InventoryId
public:	
	// Sets default values for this component's properties
	UInventoryComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	UFUNCTION()
	void OnRep_Items();
	void InitState();
public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	int32 AddItem(const UItemData& ItemData);
	FInventoryItem* GetItemByInventoryId(int32 InventoryId);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	int32 GetItemCount() const { return Items.Num(); }
	void RemoveItemByInventoryId(int32 InventoryId);
	int32 GetInventoryIdBySlot(int32 Slot);
	int32 GetFirstInventoryIdByType(EWeaponTypes ItemType);
	TArray<FInventoryItem> GetItems() const;
	bool CheckExistItem(int InventoryId);
	int32 GetMeleeId();
};
