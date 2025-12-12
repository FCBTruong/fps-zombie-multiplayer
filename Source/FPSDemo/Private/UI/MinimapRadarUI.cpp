// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/MinimapRadarUI.h"
#include <Kismet/GameplayStatics.h>
#include "Game/ActorManager.h"
#include <Components/CanvasPanelSlot.h>
#include "Game/ShooterGameState.h"
#include "Controllers/MyPlayerState.h"
#include "Game/GameManager.h"
#include "Characters/BaseCharacter.h"

void UMinimapRadarUI::NativeConstruct()
{
	Super::NativeConstruct();
	UE_LOG(LogTemp, Warning, TEXT("MinimapccRadarUI: NativeTick called"));
	
	AActorManager* AM = AActorManager::Get(GetWorld());
	if (!AM || !AM->MainPlane) return;

	AActor* MainPlane = AM->MainPlane;
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
	if (!MinimapImgPn) return;
	
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
	UpdateTeammates();
}

void UMinimapRadarUI::UpdateBombAreaLabels()
{
	const FGeometry& A_PointGeo = A_Point->GetCachedGeometry();
	const FGeometry& B_PointGeo = B_Point->GetCachedGeometry();
	FVector2D AbsPointA = A_PointGeo.GetAbsolutePosition() +
		A_PointGeo.GetLocalSize() * 0.5f;

	FVector2D AbsPointB = B_PointGeo.GetAbsolutePosition() +
		B_PointGeo.GetLocalSize() * 0.5f;

	UpdateLabelPosition(AbsPointA, A_Lb);
	UpdateLabelPosition(AbsPointB, B_Lb);
}

void UMinimapRadarUI::UpdateLabelPosition(const FVector2D& AbsPoint, UWidget * LabelWidget) {
	const FGeometry& MainGeo = MainPn->GetCachedGeometry();

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


void UMinimapRadarUI::UpdateTeammates()
{
	if (!AActorManager::Get(GetWorld())) {
		return;
	}

	APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
	if (!PC) return;
	FName MyTeamId = FName(TEXT("None"));
	AMyPlayerState* MyPS = PC->GetPlayerState<AMyPlayerState>();
	ABaseCharacter* MyPawn = PC->GetPawn<ABaseCharacter>();
	MyDot->UpdateData(true, !MyPawn->IsAlive(), MyPawn->GetWeaponComponent()->IsHasSpike());
	if (MyPS) {
		MyTeamId = MyPS->GetTeamID();
	}

	TArray<ABaseCharacter*> Players = UGameManager::Get(GetWorld())->GetRegisteredPlayers();
	for (ABaseCharacter* PawnActor : Players)
	{
		// Must have PlayerState (replicated)
		AMyPlayerState* TeamPS = PawnActor->GetPlayerState<AMyPlayerState>();
		if (!TeamPS) continue;

		// Skip self
		if (TeamPS == MyPS) continue;

		// Team filter
		if (TeamPS->GetTeamID() != MyTeamId) continue;

		// Calculate position on minimap
		FVector WorldPos = PawnActor->GetActorLocation();
		FRotator WorldRot = PawnActor->GetActorRotation();
		FVector Offset = WorldPos - WorldOrigin;
		float NormalizedX = (Offset.X + PlaneSize.X / 2) / PlaneSize.X;
		float NormalizedY = (Offset.Y + PlaneSize.Y / 2) / PlaneSize.Y;
		float MinimapX = NormalizedX * MinimapSize.X;
		float MinimapY = NormalizedY * MinimapSize.Y;
	
		UUserWidget* TeammateWidget = nullptr;
		if (TeammateWidgetsMap.Contains(TeamPS))
		{
			TeammateWidget = TeammateWidgetsMap[TeamPS];
		}
		else
		{
			// Create new widget
			TeammateWidget = CreateWidget<UUserWidget>(GetWorld(), TeammateWidgetClass);
			TeammateWidgetsMap.Add(TeamPS, TeammateWidget);

			// Add to canvas
			if (UCanvasPanel* Canvas = Cast<UCanvasPanel>(MainPn))
			{
				Canvas->AddChild(TeammateWidget);
			}
		}
		if (PawnActor->IsAlive()) {
			TeammateWidget->SetRenderTransformAngle(90 + WorldRot.Yaw + MinimapImgPn->GetRenderTransformAngle());
		}
		else {
			TeammateWidget->SetRenderTransformAngle(0.f);
		}

		UPlayerMapDot* DotWidget = Cast<UPlayerMapDot>(TeammateWidget);

		if (DotWidget)
		{
			DotWidget->UpdateData(false, !PawnActor->IsAlive(), PawnActor->GetWeaponComponent()->IsHasSpike());
		}
		// Update position
		if (auto CvSlot = Cast<UCanvasPanelSlot>(TeammateWidget->Slot))
		{
			CvSlot->SetAlignment(FVector2D(0.5f, 0.5f));
			FVector2D Des = FVector2D(MinimapX, MinimapY);
			//CvSlot->SetPosition(Des);

			auto MinimapGeo = MinimapImgPn->GetCachedGeometry();
			FVector2D AbsPoint = MinimapGeo.LocalToAbsolute(Des);
			UpdateLabelPosition(AbsPoint, TeammateWidget);
		}
	}
}