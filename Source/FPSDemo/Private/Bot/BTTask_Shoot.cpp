#include "Bot/BTTask_Shoot.h"
#include "Controllers/BotAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/BaseCharacter.h"
#include "Components/WeaponComponent.h"

UBTTask_Shoot::UBTTask_Shoot()
{
    NodeName = "Shoot Target";
    FireInterval = 0.5f; 
    LastFireTime = -100.f;
    bNotifyTick = true;
    bNotifyTaskFinished = true;
}

EBTNodeResult::Type UBTTask_Shoot::ExecuteTask(
    UBehaviorTreeComponent& OwnerComp,
    uint8* NodeMemory)
{
    return EBTNodeResult::InProgress; // We will control everything in TickTask
}

void UBTTask_Shoot::TickTask(
    UBehaviorTreeComponent& OwnerComp,
    uint8* NodeMemory,
    float DeltaSeconds)
{
    ABotAIController* AI = Cast<ABotAIController>(OwnerComp.GetAIOwner());
    if (!AI) { FinishLatentTask(OwnerComp, EBTNodeResult::Failed); return; }

    ABaseCharacter* Char = Cast<ABaseCharacter>(AI->GetPawn());
    if (!Char) { FinishLatentTask(OwnerComp, EBTNodeResult::Failed); return; }

    UWeaponComponent* WC = Char->GetWeaponComponent();
    if (!WC) { FinishLatentTask(OwnerComp, EBTNodeResult::Failed); return; }

    const float CurrentTime = AI->GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastFireTime < FireInterval)
        return;

    // Firearm logic
    if (WC->GetCurrentWeaponType() == EWeaponTypes::Firearm)
    {
        if (WC->IsReloading())
            return;

        if (WC->GetCurrentAmmoInClip() <= 0)
        {
            WC->StartReload();
            return;
        }
    }

    // Finally shoot
    WC->OnFire();
    LastFireTime = CurrentTime;

    // Keep firing forever until BT decides to exit task
}