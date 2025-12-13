// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_UpdateSpikeState.generated.h"

/**
 * 
 */
UCLASS()
class FPSDEMO_API UBTService_UpdateSpikeState : public UBTService
{
	GENERATED_BODY()

public:
    UBTService_UpdateSpikeState();
	 
protected:
    virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};
