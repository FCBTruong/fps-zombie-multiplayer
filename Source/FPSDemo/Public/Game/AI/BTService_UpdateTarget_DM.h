// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_UpdateTarget_DM.generated.h"

class ABaseCharacter;
/**
 * 
 */
UCLASS()
class FPSDEMO_API UBTService_UpdateTarget_DM : public UBTService
{
	GENERATED_BODY()
public:
	UBTService_UpdateTarget_DM();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

	void FindBestTarget(TArray<AActor*> PerceivedActors, ABaseCharacter* SelfPawn, ABaseCharacter*& OutBestTarget);
};
