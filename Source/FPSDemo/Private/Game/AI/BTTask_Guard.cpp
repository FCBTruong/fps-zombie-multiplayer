// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/AI/BTTask_Guard.h"
#include "Game/AI/BotAIController.h"


UBTTask_Guard::UBTTask_Guard() {

}

EBTNodeResult::Type UBTTask_Guard::ExecuteTask(
    UBehaviorTreeComponent& OwnerComp,
    uint8* NodeMemory)
{
    AAIController* AICon = OwnerComp.GetAIOwner();
    APawn* Pawn = AICon ? AICon->GetPawn() : nullptr;
    if (!Pawn) return EBTNodeResult::Failed;

    float YawOffset = FMath::RandRange(50.f, 90.f);
    if (FMath::RandBool())
    {
        YawOffset *= -1.f;
    }

    FRotator Rot = Pawn->GetActorRotation();
    Rot.Yaw += YawOffset;

    FVector FocusLocation =
        Pawn->GetActorLocation() + Rot.Vector() * 800.f;

    FocusLocation.Z = Pawn->GetActorLocation().Z;

    AICon->SetFocalPoint(FocusLocation);

    return EBTNodeResult::Succeeded;
}
