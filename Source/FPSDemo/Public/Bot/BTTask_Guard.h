// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_Guard.generated.h"

/**
 * 
 */
UCLASS()
class FPSDEMO_API UBTTask_Guard : public UBTTaskNode
{
	GENERATED_BODY()
	
public:
    UBTTask_Guard();

protected:
    virtual EBTNodeResult::Type ExecuteTask(
        UBehaviorTreeComponent& OwnerComp,
        uint8* NodeMemory
    ) override;
    virtual void TickTask(
        UBehaviorTreeComponent& OwnerComp,
        uint8* NodeMemory,
        float DeltaSeconds);
};
