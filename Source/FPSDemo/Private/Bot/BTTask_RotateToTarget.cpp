#include "Bot/BTTask_RotateToTarget.h"
#include "Controllers/BotAIController.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_RotateToTarget::UBTTask_RotateToTarget()
{
    NodeName = "Rotate To Target";
}

EBTNodeResult::Type UBTTask_RotateToTarget::ExecuteTask(
    UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    ABotAIController* AI = Cast<ABotAIController>(OwnerComp.GetAIOwner());
    if (!AI) return EBTNodeResult::Failed;

    APawn* Pawn = AI->GetPawn();
    UObject* Obj = AI->GetBlackboardComponent()->GetValueAsObject("TargetActor");
    AActor* Target = Cast<AActor>(Obj);

    if (!Pawn || !Target)
        return EBTNodeResult::Failed;

    FVector Dir = (Target->GetActorLocation() - Pawn->GetActorLocation()).GetSafeNormal();
    FRotator TargetRot = Dir.Rotation();

    Pawn->SetActorRotation(
        FMath::RInterpTo(
            Pawn->GetActorRotation(),
            TargetRot,
            AI->GetWorld()->GetDeltaSeconds(),
            10.f
        )
    );

    return EBTNodeResult::Succeeded;
}
