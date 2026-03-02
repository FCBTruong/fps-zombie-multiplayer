// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_PlantSpike.generated.h"

/**
 * 
 */
UCLASS()
class FPSDEMO_API UBTTask_PlantSpike : public UBTTaskNode
{
	GENERATED_BODY()
	
protected:
    virtual EBTNodeResult::Type ExecuteTask(
        UBehaviorTreeComponent& OwnerComp,
        uint8* NodeMemory
    ) override;

public:
	UBTTask_PlantSpike();
};
