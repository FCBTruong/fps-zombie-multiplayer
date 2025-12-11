// Fill out your copyright notice in the Description page of Project Settings.


#include "Bot/BTService_UpdateSpikeState.h"
#include "Controllers/BotAIController.h"
#include "Controllers/MyPlayerState.h"
#include "Game/ShooterGameState.h"
#include <BehaviorTree/BlackboardComponent.h>
#include "Game/SpikeMode.h"
#include "Characters/BaseCharacter.h"
#include "Game/GameManager.h"
#include "Pickup/PickupItem.h"
#include <Kismet/GameplayStatics.h>

UBTService_UpdateSpikeState::UBTService_UpdateSpikeState()
{
    UE_LOG(LogTemp, Log, TEXT("UBTService_UpdateSpikeState: Constructor called"));
    Interval = 0.5f;
    RandomDeviation = 0.f;
    bNotifyTick = true;
}

void UBTService_UpdateSpikeState::TickNode(
    UBehaviorTreeComponent& OwnerComp,
    uint8* NodeMemory,
    float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
    //UE_LOG(LogTemp, Log, TEXT("UBTService_UpdateSpikeState: debug03 called"));
    ABotAIController* AICon = Cast<ABotAIController>(OwnerComp.GetAIOwner());
    if (!AICon) return;

    //UE_LOG(LogTemp, Log, TEXT("UBTService_UpdateSpikeState: debug02 called"));
	UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();

    // If Is attacker
    AMyPlayerState* MyPS = AICon->GetPlayerState<AMyPlayerState>();
    if (!MyPS) return;

    AShooterGameState* GS = Cast<AShooterGameState>(AICon->GetWorld()->GetGameState());
    if (!GS) return;

 
	bool bIsAttacker = (MyPS->GetTeamID() == GS->GetAttackerTeam());

	BB->SetValueAsBool("IsAttacker", bIsAttacker);

    // get all teammates
	ASpikeMode* SpikeMode = Cast<ASpikeMode>(AICon->GetWorld()->GetAuthGameMode());
	if (!SpikeMode) return;

	TArray<AController*> Teammates = SpikeMode->GetTeamPlayers(MyPS->GetTeamID());

    bool GotSpike = false;
    for (AController* TeammateCon : Teammates)
    {
        // get pawn
		APawn* TeammatePawn = TeammateCon->GetPawn();

		// cast to BaseCharacter
		ABaseCharacter* TeammateBC = Cast<ABaseCharacter>(TeammatePawn);
		if (!TeammateBC) continue;
		// if has spike
        if (TeammateBC->GetWeaponComponent() && TeammateBC->GetWeaponComponent()->IsHasSpike())
        {
            BB->SetValueAsBool("IsTeamBringSpike", true);
            GotSpike = true;
            break;
		}
	}
    if (!GotSpike) {
        FVector SpikePos;
        //UE_LOG(LogTemp, Warning, TEXT("Object address2 = %p"), UGameManager::Instance);


        UGameManager* GM = AICon->GetWorld()
            ->GetGameInstance()
            ->GetSubsystem<UGameManager>();
        APickupItem* P = GM->GetPickupSpike();
        if (P) {
            SpikePos = P->GetActorLocation();
           
        }
        else {
            //UE_LOG(LogTemp, Log, TEXT("UBTService_UpdateSpikeState: PNULL SpikeLocation called"));
        }
        BB->SetValueAsVector("SpikeLocation", SpikePos);
    }
}