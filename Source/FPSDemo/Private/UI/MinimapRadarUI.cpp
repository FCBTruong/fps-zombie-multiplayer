// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/MinimapRadarUI.h"
#include <Kismet/GameplayStatics.h>
#include "Game/ActorManager.h"
#include <Components/CanvasPanelSlot.h>
#include "Game/ShooterGameState.h"
#include "Controllers/MyPlayerState.h"
#include "Game/GameManager.h"
#include "Characters/BaseCharacter.h"
#include "Components/InventoryComponent.h"
#include "Pickup/PickupItem.h"

void UMinimapRadarUI::NativeConstruct()
{
	Super::NativeConstruct();


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

	
	CachedPC = GetOwningPlayer();
	if (!CachedPC) return;

	// Listen to player list changes
	if (AShooterGameState* GS = Cast<AShooterGameState>(GetWorld()->GetGameState()))
	{
		GS->OnPlayerAdded.AddUObject(this, &UMinimapRadarUI::HandlePlayerAdded);
		GS->OnPlayerRemoved.AddUObject(this, &UMinimapRadarUI::HandlePlayerRemoved);

		for (APlayerState* PS : GS->PlayerArray)
		{
			HandlePlayerAdded(PS);
		}
	}
}
void UMinimapRadarUI::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	if (!MinimapImgPn) return;
	
	if (!CachedPC) return;
	PivotCurrentPawn();

	UpdateBombAreaLabels();
	UpdatePlayerDots();

	// check if spike drop on map
	auto Spike = UGameManager::Get(GetWorld())->GetPickupSpike();

	if (!SpikeIcon) return;

	if (Spike) {
		SpikeIcon->SetVisibility(ESlateVisibility::Visible);
		const FVector2D AbsSpike = WorldToMinimapAbsolute(Spike->GetActorLocation());
		ClampWidgetToMap(AbsSpike, SpikeIcon);
	}
	else {
		SpikeIcon->SetVisibility(ESlateVisibility::Hidden);
	}
}

void UMinimapRadarUI::UpdateBombAreaLabels()
{
	const FGeometry& A_PointGeo = A_Point->GetCachedGeometry();
	const FGeometry& B_PointGeo = B_Point->GetCachedGeometry();
	FVector2D AbsPointA = A_PointGeo.GetAbsolutePosition() +
		A_PointGeo.GetLocalSize() * 0.5f;

	FVector2D AbsPointB = B_PointGeo.GetAbsolutePosition() +
		B_PointGeo.GetLocalSize() * 0.5f;

	ClampWidgetToMap(AbsPointA, A_Lb);
	ClampWidgetToMap(AbsPointB, B_Lb);
}

void UMinimapRadarUI::ClampWidgetToMap(const FVector2D& AbsPoint, UWidget * LabelWidget) {
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

void UMinimapRadarUI::UpdatePlayerDots()
{
	if (!AActorManager::Get(GetWorld())) {
		return;
	}
	if (!CachedPC) return;

	ETeamId MyTeamId = ETeamId::None;
	AMyPlayerState* MyPS = CachedPC->GetPlayerState<AMyPlayerState>();
	AActor* ViewTarget = CachedPC->GetViewTarget();
	if (!ViewTarget || !MyPS) return;
	
	if (ABaseCharacter* ViewChar = Cast<ABaseCharacter>(ViewTarget))
	{
		bool bHasSpike = false;
		if (UInventoryComponent* WC = ViewChar->GetInventoryComponent())
		{
			bHasSpike = WC->HasSpike();
		}
	}

	if (MyPS) {
		MyTeamId = MyPS->GetTeamId();
	}

	for (auto& Pair : PlayerNodes)
	{
		APlayerState* PS = Pair.Key.Get();
		UPlayerMapDot* Dot = Pair.Value;

		if (!PS || !Dot) continue;

		AMyPlayerState* MyTeammatePS = Cast<AMyPlayerState>(PS);
		if (!MyTeammatePS) continue;
		if (MyTeammatePS->GetTeamId() != MyTeamId) {
			// not teammate
			Dot->SetVisibility(ESlateVisibility::Hidden);
			continue;
		}
		
		Dot->SetVisibility(ESlateVisibility::Visible);

		APawn* Pawn = PS->GetPawn();
		if (!Pawn) continue;

		// World -> minimap
		const FVector2D AbsSpike = WorldToMinimapAbsolute(Pawn->GetActorLocation());
		ClampWidgetToMap(AbsSpike, Dot);

		if (UCanvasPanelSlot* CvSlot = Cast<UCanvasPanelSlot>(Dot->Slot))
		{
			CvSlot->SetAlignment({ 0.5f, 0.5f });
		}

		float AngleYaw = 0.f;
		bool bIsSelf = false;
		if (ViewTarget == Pawn)
		{
			bIsSelf = true;
			// no need rotation
		}
		else {
			FRotator WorldRot = Pawn->GetActorRotation();
			AngleYaw = 90 + WorldRot.Yaw + MinimapImgPn->GetRenderTransformAngle();
		}

		Dot->SetRenderTransformAngle(AngleYaw);
		Dot->UpdateData(bIsSelf, false, false);
	}
}

FVector2D UMinimapRadarUI::WorldToMinimapLocal(const FVector& WorldPos) const
{
	const FVector Offset = WorldPos - WorldOrigin;

	const float NormalizedX = (Offset.X + PlaneSize.X * 0.5f) / PlaneSize.X;
	const float NormalizedY = (Offset.Y + PlaneSize.Y * 0.5f) / PlaneSize.Y;

	return FVector2D(
		NormalizedX * MinimapSize.X,
		NormalizedY * MinimapSize.Y
	);
}

FVector2D UMinimapRadarUI::WorldToMinimapAbsolute(const FVector& WorldPos) const
{
	const FVector2D Local = WorldToMinimapLocal(WorldPos);
	return MinimapImgPn->GetCachedGeometry().LocalToAbsolute(Local);
}

void UMinimapRadarUI::HandlePlayerAdded(APlayerState* NewPS)
{
	if (!NewPS || PlayerNodes.Contains(NewPS)) return;

	UPlayerMapDot* Dot = CreateWidget<UPlayerMapDot>(GetWorld(), TeammateWidgetClass);
	if (!Dot) return;

	UE_LOG(LogTemp, Log, TEXT("MinimapRadarUI::HandlePlayerAdded: %s"), *NewPS->GetPlayerName());
	MainPn->AddChild(Dot);
	PlayerNodes.Add(NewPS, Dot);
}

void UMinimapRadarUI::HandlePlayerRemoved(APlayerState* RemovedPS)
{
	if (UPlayerMapDot** DotPtr = PlayerNodes.Find(RemovedPS))
	{
		if (*DotPtr)
		{
			(*DotPtr)->RemoveFromParent();
		}
		PlayerNodes.Remove(RemovedPS);
	}
}

void UMinimapRadarUI::PivotCurrentPawn()
{
	AActor* ViewTarget = CachedPC->GetViewTarget();
	if (!IsValid(ViewTarget)) return;

	FVector WorldPos = ViewTarget->GetActorLocation();
	FVector Offset = WorldPos - WorldOrigin;

	float NormalizedX = (Offset.X + PlaneSize.X / 2) / PlaneSize.X;
	float NormalizedY = (Offset.Y + PlaneSize.Y / 2) / PlaneSize.Y;

	float YawValue = FRotator::NormalizeAxis(ViewTarget->GetActorRotation().Yaw);
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
}