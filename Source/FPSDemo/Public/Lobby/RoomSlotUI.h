// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Lobby/RoomData.h"
#include "Components/Button.h"
#include "RoomSlotUI.generated.h"

/**
 * 
 */
UCLASS()
class FPSDEMO_API URoomSlotUI : public UUserWidget
{
	GENERATED_BODY()
	
public:
	void Init(FRoomData Data);

	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	UButton* JoinBtn;

private:
	UFUNCTION()
	void OnJoinBtnClicked();
	FRoomData CachedRoomData;
};
