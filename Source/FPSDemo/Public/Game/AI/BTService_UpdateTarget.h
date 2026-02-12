// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Services/BTService_BlueprintBase.h"
#include "BTService_UpdateTarget.generated.h"

/**
 * 
 */
UCLASS()
class FPSDEMO_API UBTService_UpdateTarget : public UBTService_BlueprintBase
{
    GENERATED_BODY()

public:
    UBTService_UpdateTarget();

protected:
    virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
};