// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/TeamEliminationState.h"
#include "Net/UnrealNetwork.h"


void ATeamEliminationState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    DOREPLIFETIME(ATeamEliminationState, TeamAScore);
    DOREPLIFETIME(ATeamEliminationState, TeamBScore);
}

void ATeamEliminationState::OnRep_Score()
{
	OnUpdateScore.Broadcast();
	UE_LOG(LogTemp, Warning, TEXT("Scores updated: Team A: %d, Team B: %d"), TeamAScore, TeamBScore);
}

