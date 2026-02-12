// ZoomableButton.cpp

#include "Game/UI/ZoomableButton.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "Game/GameManager.h"
#include "Shared/Data/GlobalDataAsset.h"

void UZoomableButton::SynchronizeProperties()
{
    Super::SynchronizeProperties();

    SetRenderTransformPivot(FVector2D(0.5f, 0.5f));

    OnPressed.Clear();
    OnReleased.Clear();

    OnPressed.AddDynamic(this, &UZoomableButton::HandlePressed);
    OnReleased.AddDynamic(this, &UZoomableButton::HandleReleased);
}

void UZoomableButton::HandlePressed()
{
    // play sound
	UGameManager* GMR = UGameManager::Get(GetWorld());
    if (GMR && GMR->GlobalData && GMR->GlobalData->TouchSound)
    {
        UGameplayStatics::PlaySound2D(this, GMR->GlobalData->TouchSound);
	}
    PlayZoomTo(0.9f);
}

void UZoomableButton::HandleReleased()
{
    PlayZoomTo(1.0f);
}

void UZoomableButton::PlayZoomTo(float NewTargetScale)
{
    if (UWorld* World = GetWorld())
    {
        World->GetTimerManager().ClearTimer(ZoomTimerHandle);

        AnimElapsed = 0.0f;
        StartScale = CurrentScale;
        TargetScale = NewTargetScale;

        const float TickInterval = 1.0f / 60.0f;

        World->GetTimerManager().SetTimer(
            ZoomTimerHandle,
            FTimerDelegate::CreateUObject(this, &UZoomableButton::UpdateZoom, TickInterval),
            TickInterval,
            true
        );
    }
}

void UZoomableButton::UpdateZoom(float TickInterval)
{
    AnimElapsed += TickInterval;

    const float Alpha = FMath::Clamp(AnimElapsed / AnimDuration, 0.0f, 1.0f);
    const float NewScale = FMath::Lerp(StartScale, TargetScale, Alpha);

    SetRenderScale(FVector2D(NewScale, NewScale));
    CurrentScale = NewScale;

    if (Alpha >= 1.0f)
    {
        if (UWorld* World = GetWorld())
        {
            World->GetTimerManager().ClearTimer(ZoomTimerHandle);
        }
    }
}
