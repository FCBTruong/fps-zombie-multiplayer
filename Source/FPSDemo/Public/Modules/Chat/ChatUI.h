// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/EditableText.h"
#include "Components/ScrollBox.h"
#include "ChatUI.generated.h"

class UChatNode;
class UChatSubsystem;
/**
 * 
 */
UCLASS()
class FPSDEMO_API UChatUI : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION()
	void OpenChat();

	UFUNCTION()
	void CloseChat();
	void ToggleChat();

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(EditDefaultsOnly, Category = "Chat")
	TSubclassOf<UChatNode> ChatNodeClass;

	UPROPERTY(meta = (BindWidget))
	UWidget* ChatPreviewPn;

	UPROPERTY(meta = (BindWidget))
	UWidget* ChatPn;

	UPROPERTY(meta = (BindWidget))
	UEditableText* ChatInput;

	UPROPERTY(meta = (BindWidget))
	UScrollBox* ChatScrollBox;

	UPROPERTY(meta = (BindWidget))
	UScrollBox* ChatPreviewScrollBox;

private:
	bool bIsChatOpen = false;
	bool bIsLastMouseCursor = false;

	UFUNCTION()
	void OnChatTextCommitted(const FText& Text, ETextCommit::Type CommitMethod);

	void HandleNewChatMessage(const FString& Sender, const FString& Message);

	void SetChatOpen(bool bOpen);
	UChatSubsystem* ChatSubsystem = nullptr;
};
