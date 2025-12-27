// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/SpikeComponent.h"

// Sets default values for this component's properties
USpikeComponent::USpikeComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void USpikeComponent::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Log, TEXT("USpikeComponent::BeginPlay called"));
	// ...
	
}


// Called every frame
void USpikeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

