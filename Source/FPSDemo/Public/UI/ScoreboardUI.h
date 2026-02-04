// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI/ScoreboardSlotUI.h"
#include "Components/VerticalBox.h"
#include "ScoreboardUI.generated.h"

/**
 * 
 */
UCLASS()
class FPSDEMO_API UScoreboardUI : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(meta = (BindWidget))
	class UWidget* Divide;

	UPROPERTY(meta = (BindWidget))
	class UVerticalBox* ScoreboardBox;

	UPROPERTY(meta = (BindWidget))
	class UVerticalBox* ScoreboardBoxA;

	UPROPERTY(meta = (BindWidget))
	class UVerticalBox* ScoreboardBoxB;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UScoreboardSlotUI> ScoreboardSlotClass;

public:
	void NativeConstruct() override;
	void UpdateScoreboard(class AShooterGameState* GameState);
};
