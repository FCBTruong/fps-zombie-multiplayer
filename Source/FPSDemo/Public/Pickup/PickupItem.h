// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "PickupData.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "PickupItem.generated.h"

UCLASS()
class FPSDEMO_API APickupItem : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	APickupItem();

private:
	FPickupData Data;
	UPROPERTY()
	USkeletalMeshComponent* ItemMesh;
	UPROPERTY()
	USphereComponent* PickupSphere;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	void SetData(const FPickupData& NewData);
	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	FORCEINLINE FPickupData GetData() const { return Data; }
};
