// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/MinimapRadarUI.h"
#include <Kismet/GameplayStatics.h>
#include "Game/ActorManager.h"
#include <Components/CanvasPanelSlot.h>

void UMinimapRadarUI::NativeConstruct()
{
	Super::NativeConstruct();
	UE_LOG(LogTemp, Warning, TEXT("MinimapccRadarUI: NativeTick called"));
	
	if (!AActorManager::Instance) {
		return;
	}
	if (!AActorManager::Instance->MainPlane) {
		return;
	}
	AActor* MainPlane = AActorManager::Instance->MainPlane;
	MainPlane->GetActorBounds(true, WorldOrigin, WorldExtent);

	PlaneSize = WorldExtent * 2.f;
	UCanvasPanelSlot* CvSlot = Cast<UCanvasPanelSlot>(MinimapImage->Slot);
	if (CvSlot)
	{
		MinimapSize = CvSlot->GetSize();
	}
	// log minimap size
	UE_LOG(LogTemp, Warning, TEXT("MinimapccRadarUI Size: X=%f, Y=%f"), MinimapSize.X, MinimapSize.Y);
}
void UMinimapRadarUI::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	if (!MinimapImage || !Dot) return;
	
	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (!PC) return;
	APawn* Pawn = PC->GetPawn();
	if (!Pawn) return;

	FVector WorldPos = Pawn->GetActorLocation();
	FVector Offset = WorldPos - WorldOrigin;

	float NormalizedX = (Offset.X + PlaneSize.X / 2) / PlaneSize.X;
	float NormalizedY = (Offset.Y + PlaneSize.Y / 2) / PlaneSize.Y;
	
	float YawValue = FRotator::NormalizeAxis(Pawn->GetActorRotation().Yaw);
	FWidgetTransform T;
	T.Angle = -90 - YawValue;

	float PivotX = NormalizedX;
	float PivotY = NormalizedY;
	FVector2D NewPivot(PivotX, PivotY);
	FVector2D OldPivot = MinimapImage->GetRenderTransformPivot();
	MinimapImage->SetRenderTransformPivot(NewPivot);
	FVector2D OffsetPv = (OldPivot - NewPivot) * MinimapSize;
	if (auto CanvasSlot = Cast<UCanvasPanelSlot>(MinimapImage->Slot))
	{
		CanvasSlot->SetPosition(CanvasSlot->GetPosition() + OffsetPv);
	}
	MinimapImage->SetRenderTransform(T);
}