// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/Framework/DedicatedServerMode.h"
#include "Engine/NetDriver.h"
void ADedicatedServerMode::BeginPlay()
{
	Super::BeginPlay();

	// log tick number for testing
    if (UWorld* World = GetWorld())
    {
        if (UNetDriver* NetDriver = World->GetNetDriver())
        {
            UE_LOG(LogTemp, Log, TEXT("NetServerMaxTickRate = %d"), NetDriver->NetServerMaxTickRate);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("No NetDriver found"));
        }
    }
}
