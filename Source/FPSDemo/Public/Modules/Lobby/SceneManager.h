// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SceneManager.generated.h"

/**
 * 
 */
UCLASS()
class FPSDEMO_API USceneManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()
public:
	static USceneManager* Get(UWorld* World);

	void OpenPopupDialogOkCancel(
		const FString& Message,
		TFunction<void()> OnOk,
		TFunction<void()> OnCancel
	);
};
