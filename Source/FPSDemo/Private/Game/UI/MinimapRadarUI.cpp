// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/UI/MinimapRadarUI.h"
#include <Kismet/GameplayStatics.h>
#include "Game/Subsystems/ActorManager.h"
#include <Components/CanvasPanelSlot.h>
#include "Game/Framework/ShooterGameState.h"
#include "Game/Framework/MyPlayerState.h"
#include "Game/GameManager.h"
#include "Game/Characters/BaseCharacter.h"
#include "Game/Characters/Components/InventoryComponent.h"
#include "Game/Items/Pickup/PickupItem.h"

void UMinimapRadarUI::NativeConstruct()
{
	Super::NativeConstruct();
	UE_LOG(LogTemp, Warning, TEXT("MinimapRadarUI: NativeConstruct called"));

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

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			PeriodicUpdateTimerHandle,
			this,
			&UMinimapRadarUI::PeriodicUpdate,
			1.0f,
			true
		);
		PeriodicUpdate();
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

	UpdateSpike();
}

void UMinimapRadarUI::PeriodicUpdate()
{
	if (!CachedPC) return;
	AMyPlayerState* MyPS = CachedPC->GetPlayerState<AMyPlayerState>();
	ETeamId MyTeamId = ETeamId::None;
	if (MyPS) {
		MyTeamId = MyPS->GetTeamId();
	}
	TMap<ABaseCharacter*, UPlayerMapDot*> Enemies;
	Enemies.Reserve(PlayerNodes.Num());
	for (auto& Pair : PlayerNodes)
	{
		UPlayerMapDot* Dot = Pair.Value;
		Dot->SetVisibility(ESlateVisibility::Hidden);

		AMyPlayerState* PS = Pair.Key.Get();
		if (!PS) continue;
		if (PS->GetTeamId() != MyTeamId) {
			ABaseCharacter* EnemyPawn = Cast<ABaseCharacter>(PS->GetPawn());
			if (EnemyPawn)
			{
				Enemies.Add(EnemyPawn, Dot);
			}
		}
		else {
			ABaseCharacter* Char = Cast<ABaseCharacter>(PS->GetPawn());
			bool bIsSelf = (Char == CachedPC->GetViewTarget());
			if (!Char) continue;
			bool bHasSpike = false;

			if (UInventoryComponent* WC = Char->GetInventoryComponent())
			{
				bHasSpike = WC->HasSpike();
			}
			Dot->UpdateData(bIsSelf, Char->IsDead(), bHasSpike);
		}
		Dot->SetIsTeammateVisual(PS->GetTeamId() == MyTeamId);
	}
	for (auto& Pair : PlayerNodes)
	{
		UPlayerMapDot* Dot = Pair.Value;
		if (!Dot) continue;

		AMyPlayerState* PS = Pair.Key.Get();
		if (!PS) continue;

		if (PS->GetTeamId() == MyTeamId) {
			Dot->SetVisibility(ESlateVisibility::Visible);

			ABaseCharacter* Char = Cast<ABaseCharacter>(PS->GetPawn());
			if (!Char) continue;
			if (Char->IsDead()) continue;
			// check if it can see
			for (auto& EnemyPair : Enemies)
			{
				ABaseCharacter* EnemyPawn = EnemyPair.Key;
				UPlayerMapDot* EnemyDot = EnemyPair.Value;
				if (!EnemyPawn || !EnemyDot) continue;
				if (EnemyPawn->IsDead()) continue;

				if (Char->CanSeeThisActor(EnemyPawn))
				{
					EnemyDot->SetVisibility(ESlateVisibility::Visible);
				}
			}
		}
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

void UMinimapRadarUI::UpdateSpike()
{
	// handled in NativeTick
	// check if spike drop on map
	auto Spike = UGameManager::Get(GetWorld())->GetPickupSpike();

	if (!SpikeIcon) return;

	AMyPlayerState* MyPS = CachedPC->GetPlayerState<AMyPlayerState>();
	if (!MyPS) return;

	SpikeIcon->SetVisibility(ESlateVisibility::Hidden);
	if (MyPS->GetTeamId() != ETeamId::Attacker)
	{
		return; // only attacker can see spike on minimap
	}

	if (Spike) {
		SpikeIcon->SetVisibility(ESlateVisibility::Visible);
		const FVector2D AbsSpike = WorldToMinimapAbsolute(Spike->GetActorLocation());
		ClampWidgetToMap(AbsSpike, SpikeIcon);
	}
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

	AActor* ViewTarget = CachedPC->GetViewTarget();
	if (!ViewTarget) return;

	for (auto& Pair : PlayerNodes)
	{
		AMyPlayerState* PS = Pair.Key.Get();
		UPlayerMapDot* Dot = Pair.Value;

		if (!PS || !Dot) continue;

		APawn* Pawn = PS->GetPawn();
		if (!Pawn) continue;
		ABaseCharacter* Char = Cast<ABaseCharacter>(Pawn);
		if (!Char) continue;

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

void UMinimapRadarUI::HandlePlayerAdded(APlayerState* PS)
{
	AMyPlayerState* NewPS = Cast<AMyPlayerState>(PS);
	if (!NewPS || PlayerNodes.Contains(NewPS)) return;

	UPlayerMapDot* Dot = CreateWidget<UPlayerMapDot>(GetWorld(), TeammateWidgetClass);
	if (!Dot) return;

	UE_LOG(LogTemp, Log, TEXT("MinimapRadarUI::HandlePlayerAdded: %s"), *NewPS->GetPlayerName());
	MainPn->AddChild(Dot);
	PlayerNodes.Add(NewPS, Dot);
}

void UMinimapRadarUI::HandlePlayerRemoved(APlayerState* PS)
{
	AMyPlayerState* RemovedPS = Cast<AMyPlayerState>(PS);
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
	ABaseCharacter* ViewChar = Cast<ABaseCharacter>(ViewTarget);
	if (!IsValid(ViewChar)) return;
	if (ViewChar->IsDead()) return;

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