// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/PlayerSlot.h"
#include "Net/UnrealNetwork.h"

// Sets default values
APlayerSlot::APlayerSlot()
{
	USceneComponent* SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = SceneRoot;
	bAlwaysRelevant = true;

	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
}

void APlayerSlot::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(APlayerSlot, BackendUserId);
	DOREPLIFETIME(APlayerSlot, TeamId);
	DOREPLIFETIME(APlayerSlot, bIsBot);
	DOREPLIFETIME(APlayerSlot, bIsConnected);
	DOREPLIFETIME(APlayerSlot, Kills);
	DOREPLIFETIME(APlayerSlot, Deaths);
	DOREPLIFETIME(APlayerSlot, Assists);
	DOREPLIFETIME(APlayerSlot, Pawn);
}

void APlayerSlot::OnRep_Pawn()
{
	OnReplicatedPawnChanged.Broadcast();
}

void APlayerSlot::SetPawn(APawn* InPawn)
{
	if (Pawn != InPawn)
	{
		Pawn = InPawn;
		OnRep_Pawn();
	}
}

void APlayerSlot::SetIsConnected(bool bInIsConnected)
{
	if (bIsConnected != bInIsConnected)
	{
		bIsConnected = bInIsConnected;
		OnRep_IsConnected();
	}
}

void APlayerSlot::OnRep_IsConnected()
{
	OnUpdateConnectedStatus.Broadcast(bIsConnected);
}