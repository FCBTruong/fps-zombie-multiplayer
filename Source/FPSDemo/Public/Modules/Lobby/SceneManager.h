// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Network/PacketListener.h"
#include "SceneManager.generated.h"

/**
 * 
 */
UCLASS()
class FPSDEMO_API USceneManager : public UGameInstanceSubsystem, public IPacketListener
{
	GENERATED_BODY()
public:
	static USceneManager* Get(UWorld* World);
	void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void OnPacketReceived(
		ECmdId CmdId,
		const std::string& Payload
	) override;

	void OpenPopupDialogOkCancel(
		const FString& Message,
		TFunction<void()> OnOk,
		TFunction<void()> OnCancel
	);
	void OpenPopupDialogOk(
		const FString& Message,
		TFunction<void()> OnOk
	);
};
