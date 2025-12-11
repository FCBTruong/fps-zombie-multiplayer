// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/MinimapRadarUI.h"
#include <Kismet/GameplayStatics.h>
#include "Game/ActorManager.h"
#include <Components/CanvasPanelSlot.h>

void UMinimapRadarUI::NativeConstruct()
{
	Super::NativeConstruct();
	UE_LOG(LogTemp, Warning, TEXT("MinimapccRadarUI: NativeTick called"));
	
	if (!AActorManager::Get(GetWorld())) {
		return;
	}
	if (!AActorManager::Get(GetWorld())->MainPlane) {
		return;
	}
	AActor* MainPlane = AActorManager::Get(GetWorld())->MainPlane;
	MainPlane->GetActorBounds(true, WorldOrigin, WorldExtent);

	PlaneSize = WorldExtent * 2.f;
	UCanvasPanelSlot* CvSlot = Cast<UCanvasPanelSlot>(MinimapImgPn->Slot);
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
	if (!MinimapImgPn || !Dot) return;
	
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
	FVector2D OldPivot = MinimapImgPn->GetRenderTransformPivot();
	MinimapImgPn->SetRenderTransformPivot(NewPivot);
	FVector2D OffsetPv = (OldPivot - NewPivot) * MinimapSize;
	if (auto CanvasSlot = Cast<UCanvasPanelSlot>(MinimapImgPn->Slot))
	{
		CanvasSlot->SetPosition(CanvasSlot->GetPosition() + OffsetPv);
	}
	MinimapImgPn->SetRenderTransform(T);

	UpdateBombAreaLabels();
}

void UMinimapRadarUI::UpdateBombAreaLabels()
{
	UpdateLabelPosition(A_Point, A_Lb);
	UpdateLabelPosition(B_Point, B_Lb);
}

void UMinimapRadarUI::UpdateLabelPosition(UWidget * PointWidget, UWidget * LabelWidget) {
	const FGeometry& MainGeo = MainPn->GetCachedGeometry();
	const FGeometry& PointGeo = PointWidget->GetCachedGeometry();

	// 1) Get A_Point absolute center
	FVector2D AbsPoint = PointGeo.GetAbsolutePosition() +
		PointGeo.GetLocalSize() * 0.5f;

	// 2) Convert to MainPn local space
	FVector2D LocalPoint = MainGeo.AbsoluteToLocal(AbsPoint);

	// ----- circle data -----
	FVector2D CircleSize = MainGeo.GetLocalSize();
	float Radius = CircleSize.X * 0.5f;
	FVector2D Center(Radius, Radius);

	FVector2D Dir = LocalPoint - Center;
	float Dist = Dir.Size();
	FVector2D FinalLocal;

	if (Dist <= Radius)
	{
		// inside circle
		FinalLocal = LocalPoint;
	}
	else
	{
		// clamp to border
		FinalLocal = Center + (Dir / Dist) * Radius;
	}

	if (auto CvSlot = Cast<UCanvasPanelSlot>(LabelWidget->Slot))
	{
		CvSlot->SetPosition(FinalLocal);
	}
}
