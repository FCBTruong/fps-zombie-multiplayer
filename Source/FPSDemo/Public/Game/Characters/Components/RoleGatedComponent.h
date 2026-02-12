// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RoleGatedComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FPSDEMO_API URoleGatedComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	URoleGatedComponent();

    void SetEnabled(bool bInEnabled);

    bool IsEnabled() const { return bEnabled; }

private:
    UPROPERTY()
    bool bEnabled = true;

protected:
    // override in derived components to clear timers/reset state
    virtual void OnEnabledChanged(bool bNowEnabled) {}
};
