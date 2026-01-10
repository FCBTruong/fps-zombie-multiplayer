#include "Bot/BTTask_ZombieAttack.h"
#include "Controllers/BotAIController.h"
#include "Characters/BaseCharacter.h"
#include "Components/EquipComponent.h"
#include "Components/WeaponFireComponent.h"

UBTTask_ZombieAttack::UBTTask_ZombieAttack()
{
    NodeName = "Zomibe Attack Target";
}

EBTNodeResult::Type UBTTask_ZombieAttack::ExecuteTask(
    UBehaviorTreeComponent& OwnerComp,
    uint8* NodeMemory)
{
    UE_LOG(LogTemp, Log, TEXT("UBTTask_ZombieAttack: fire called"));

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

	Char->RequestPrimaryActionPressed();

    return EBTNodeResult::Succeeded;
}

