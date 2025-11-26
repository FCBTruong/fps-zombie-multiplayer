#pragma once

#include "CoreMinimal.h"
#include "Components/Button.h"
#include "ZoomableButton.generated.h"

UCLASS()
class FPSDEMO_API UZoomableButton : public UButton
{
    GENERATED_BODY()

public:
    virtual void SynchronizeProperties() override;

protected:
    UFUNCTION()
    void HandlePressed();

    UFUNCTION()
    void HandleReleased();

    void PlayZoomTo(float NewTargetScale);
    void UpdateZoom(float TickInterval);

private:
    FTimerHandle ZoomTimerHandle;

    float AnimElapsed = 0.0f;
    float AnimDuration = 0.08f;   // animation time per transition
    float StartScale = 1.0f;
    float TargetScale = 1.0f;
    float CurrentScale = 1.0f;
};