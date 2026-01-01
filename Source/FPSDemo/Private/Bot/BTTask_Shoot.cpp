#include "Bot/BTTask_Shoot.h"
#include "Controllers/BotAIController.h"
#include "Characters/BaseCharacter.h"
#include "Components/EquipComponent.h"
#include "Components/WeaponFireComponent.h"

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

    UEquipComponent* EC = Char->GetEquipComponent();
    if (!EC)
    {
        return EBTNodeResult::Failed;
    }

    // === Target ===
	APawn* Target = AI->GetTargetActor();
    if (!Target)
    {
        return EBTNodeResult::Failed;
    }
    //AI->SetFocus(Target);

    // check Has Sight
	bool IsHasSight = AI->HasLineOfSight();
    if (!IsHasSight) {
		return EBTNodeResult::Succeeded;
    }

    // === Base aim ===
    const FVector CameraLocation = Char->GetPawnViewLocation();

    FVector TargetPoint = Target->GetActorLocation();
    TargetPoint.Z += 60.f; // chest height


    // === Fire ===
    AI->RequestFireOnce();


    return EBTNodeResult::Succeeded;
}

