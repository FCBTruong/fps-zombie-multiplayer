// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Image.h"
#include "Blueprint/UserWidget.h"
#include "Components/CanvasPanel.h"
#include "UI/PlayerMapDot.h"
#include "MinimapRadarUI.generated.h"

class AMyPlayerState;
/**
 * 
 */
UCLASS()
class FPSDEMO_API UMinimapRadarUI : public UUserWidget
{
	GENERATED_BODY()

protected:

	UPROPERTY(meta = (BindWidget))
	class UCanvasPanel* MinimapImgPn;

	UPROPERTY(meta = (BindWidget))
	class UCanvasPanel* MainPn;

	UPROPERTY(meta = (BindWidget))
	class UCanvasPanel* B_Point;
	UPROPERTY(meta = (BindWidget))
	class UCanvasPanel* A_Point;

	FVector WorldOrigin;
	FVector WorldExtent;
	FVector PlaneSize;
	FVector2D MinimapSize;

	UPROPERTY(meta = (BindWidget))
	UWidget* A_Lb;
	UPROPERTY(meta = (BindWidget))
	UWidget* B_Lb;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Teammate")
	TSubclassOf<UUserWidget> TeammateWidgetClass;

	TMap<APlayerState*, UUserWidget*> TeammateWidgetsMap;

	UPROPERTY(meta = (BindWidget))
	UWidget* SpikeIcon;

private:
	FTimerHandle PeriodicUpdateTimerHandle;
	UFUNCTION()
	void PeriodicUpdate();
public:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual void NativeConstruct() override;

	void UpdateBombAreaLabels();
	void ClampWidgetToMap(const FVector2D& AbsPoint, UWidget* LabelWidget);
	void UpdatePlayerDots();
	void UpdateSpike();
	FVector2D WorldToMinimapAbsolute(const FVector& WorldPos) const;
	FVector2D WorldToMinimapLocal(const FVector& WorldPos) const;

	UPROPERTY()
	TMap<TWeakObjectPtr<AMyPlayerState>, UPlayerMapDot*> PlayerNodes;

	UPROPERTY()
	APlayerController* CachedPC;

	UFUNCTION()
	void HandlePlayerAdded(APlayerState* NewPlayerState);

	UFUNCTION()
	void HandlePlayerRemoved(APlayerState* RemovedPlayerState);
	void PivotCurrentPawn();
};
