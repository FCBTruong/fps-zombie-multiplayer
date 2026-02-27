// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AirdropCrate.generated.h"

class ABaseCharacter;
UCLASS()
class FPSDEMO_API AAirdropCrate : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAirdropCrate();
	void SetClaimed(bool bNewIsClaimed) { bIsClaimed = bNewIsClaimed; }
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	UStaticMeshComponent* Mesh;
	bool bIsClaimed;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);
};
