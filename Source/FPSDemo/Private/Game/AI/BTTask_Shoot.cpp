#include "Game/AI/BTTask_Shoot.h"
#include "Game/AI/BotAIController.h"
#include "Game/Characters/BaseCharacter.h"

UBTTask_Shoot::UBTTask_Shoot()
{
    NodeName = "Shoot Target";
}

EBTNodeResult::Type UBTTask_Shoot::ExecuteTask(
    UBehaviorTreeComponent& OwnerComp,
    uint8* NodeMemory)
{
    UE_LOG(LogTemp, Log, TEXT("UBTTask_Shoot: fire called"));

    // === AI & Pawn ===
    ABotAIController* AI = Cast<ABotAIController>(OwnerComp.GetAIOwner());
    if (!AI)
    {
        return EBTNodeResult::Failed;
    }
    
    ABaseCharacter* Char = Cast<ABaseCharacter>(AI->GetPawn());
    if (!Char)
    {
        return EBTNodeResult::Failed;
    }

    // === Target ===
    ABaseCharacter* Target = AI->GetTargetActor();
    if (!Target)
    {
        return EBTNodeResult::Failed;
    }
    //AI->SetFocus(Target);

    // check Has Sight
	/*bool IsHasSight = AI->HasLineOfSight();
    if (!IsHasSight) {
		return EBTNodeResult::Succeeded;
    }*/

    // === Fire ===
    AI->RequestFireOnce();

    return EBTNodeResult::Succeeded;
}

