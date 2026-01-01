#include "Bot/BTTask_RotateToTarget.h"
#include "Controllers/BotAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/BaseCharacter.h"

UBTTask_RotateToTarget::UBTTask_RotateToTarget()
{
    NodeName = "Rotate To Target";
    bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_RotateToTarget::ExecuteTask(
    UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    return EBTNodeResult::InProgress;
}

void UBTTask_RotateToTarget::TickTask(
    UBehaviorTreeComponent& OwnerComp,
    uint8* NodeMemory,
    float DeltaSeconds)
{
    ABotAIController* AI = Cast<ABotAIController>(OwnerComp.GetAIOwner());
    if (!AI) { FinishLatentTask(OwnerComp, EBTNodeResult::Failed); return; }

    APawn* Pawn = AI->GetPawn();
	ABaseCharacter* Target = AI->GetTargetActor();

    if (!Pawn || !Target)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    FVector Dir = (Target->GetActorLocation() - Pawn->GetActorLocation()).GetSafeNormal();
    FRotator TargetRot = Dir.Rotation();

    FRotator CurrentRot = AI->GetControlRotation();

    FRotator NewRot = FMath::RInterpTo(
        CurrentRot,
        TargetRot,
        DeltaSeconds,
        RotateSpeed
    );

    AI->SetControlRotation(NewRot);

    float DeltaYaw = FMath::Abs(FMath::FindDeltaAngleDegrees(NewRot.Yaw, TargetRot.Yaw));

    UE_LOG(LogTemp, Warning,
        TEXT("RotateTask: Current=%.1f Target=%.1f New=%.1f Delta=%.2f"),
        CurrentRot.Yaw, TargetRot.Yaw, NewRot.Yaw, DeltaYaw);

    if (DeltaYaw < 1.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("RotateTask: Finished"));
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
        return;
    }
}

void UBTTask_RotateToTarget::OnTaskFinished(
    UBehaviorTreeComponent& OwnerComp,
    uint8* NodeMemory,
    EBTNodeResult::Type TaskResult)
{
    UE_LOG(LogTemp, Warning, TEXT("RotateTask FINISHED with result = %d"), (int)TaskResult);
}
