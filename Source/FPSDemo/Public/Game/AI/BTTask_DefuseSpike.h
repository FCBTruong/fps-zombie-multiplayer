// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_DefuseSpike.generated.h"

/**
 * 
 */
UCLASS()
class FPSDEMO_API UBTTask_DefuseSpike : public UBTTaskNode
{
	GENERATED_BODY()

public:
    UBTTask_DefuseSpike();

protected:
    virtual EBTNodeResult::Type ExecuteTask(
        UBehaviorTreeComponent& OwnerComp,
        uint8* NodeMemory
    ) override;
};
