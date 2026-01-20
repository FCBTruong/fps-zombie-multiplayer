// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "ChatNode.generated.h"

/**
 * 
 */
UCLASS()
class FPSDEMO_API UChatNode : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetChatMessage(const FString& Sender, const FString& Message);

protected:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* SenderLb;
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* MessageLb;
};
