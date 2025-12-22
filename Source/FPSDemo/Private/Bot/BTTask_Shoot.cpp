#include "Bot/BTTask_Shoot.h"
#include "Controllers/BotAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Characters/BaseCharacter.h"
#include "Components/WeaponComponent.h"

UBTTask_Shoot::UBTTask_Shoot()
{
    NodeName = "Shoot Target";
    bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_Shoot::ExecuteTask(
    UBehaviorTreeComponent& OwnerComp,
    uint8* NodeMemory)
{
	// Always return InProgress, actual shooting logic will be handled in TickTask
    return EBTNodeResult::InProgress;
}

void UBTTask_Shoot::TickTask(
    UBehaviorTreeComponent& OwnerComp,
    uint8* NodeMemory,
    float DeltaSeconds)
{
    // === Fire rate control ===
    const float CurrentTime = GetWorld()->GetTimeSeconds();
    const float FireInterval = 0.3f;

    if (CurrentTime - LastTimeFire < FireInterval)
    {
        return;
    }
    UE_LOG(LogTemp, Log, TEXT("UBTTask_Shoot: fire called"));

    // === AI & Pawn ===
    ABotAIController* AI = Cast<ABotAIController>(OwnerComp.GetAIOwner());
    if (!AI)
    {
        return;
    }

    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    ABaseCharacter* Char = Cast<ABaseCharacter>(AI->GetPawn());
    if (!Char || !BB)
    {
        return;
    }

    UWeaponComponent* WC = Char->GetWeaponComponent();
    if (!WC)
    {
        return;
    }

    // === Target ===
    APawn* Target = Cast<APawn>(BB->GetValueAsObject(TEXT("Obj_TargetActor")));
    if (!Target)
    {
        return;
    }

    // check Has Sight
    bool IsHasSight = BB->GetValueAsBool(TEXT("B_HasLineSight"));
    if (!IsHasSight) {
        return;
    }

    // === Base aim ===
    const FVector CameraLocation = Char->GetPawnViewLocation();

    FVector TargetPoint = Target->GetActorLocation();
    TargetPoint.Z += 60.f; // chest height

    // === Distance-based accuracy ===
    const float Distance = FVector::Dist(CameraLocation, TargetPoint);

    float AimError = FMath::GetMappedRangeValueClamped(
        FVector2D(300.f, 2000.f),
        FVector2D(10.f, 60.f), // cm
        Distance
    );

    // === Movement penalty ===
    const float TargetSpeed = Target->GetVelocity().Size();
    AimError += TargetSpeed * 0.015f;

    // === Random spread ===
    TargetPoint += FMath::VRand() * AimError;


    // === Fire ===
    WC->RequestStartFire();

    LastTimeFire = CurrentTime;
}
