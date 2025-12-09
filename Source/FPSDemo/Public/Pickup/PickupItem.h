// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PickupData.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "PickupItem.generated.h"

class ABaseCharacter;
UCLASS()
class FPSDEMO_API APickupItem : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APickupItem();

private:
	UPROPERTY(ReplicatedUsing = OnRep_Data)
	FPickupData Data;
	UPROPERTY()
	UStaticMeshComponent* ItemMesh;
	UPROPERTY()
	USphereComponent* PickupSphere;
	double LastDropTimeMs = 0;
	ABaseCharacter* LastOwner = nullptr;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnRep_Data();
	void OnLoadData();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	void SetData(const FPickupData& NewData);
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	FORCEINLINE FPickupData GetData() const { return Data; }
	FORCEINLINE UStaticMeshComponent* GetItemMesh() const { return ItemMesh; }
	FString GetItemName() const;

	FPickupData GetPickupData() const { return Data; }
	void PlayerDropInfo(ABaseCharacter* Character);
	bool IsJustDropped(ABaseCharacter* Character) const;
};
