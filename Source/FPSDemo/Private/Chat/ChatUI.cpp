// Fill out your copyright notice in the Description page of Project Settings.


#include "Chat/ChatUI.h"
#include "Chat/ChatNode.h"
#include "Chat/ChatSubsystem.h"

void UChatUI::NativeConstruct()
{
	Super::NativeConstruct();
	if (ChatSubsystem == nullptr)
	{
		ChatSubsystem = GetGameInstance()->GetSubsystem<UChatSubsystem>();
	}
	if (ChatSubsystem)
	{
		ChatSubsystem->OnNewChatMessage.AddUObject(this, &UChatUI::HandleNewChatMessage);
	}
	if (ChatInput)
	{
		ChatInput->OnTextCommitted.AddDynamic(this, &UChatUI::OnChatTextCommitted);
	}
	bIsChatOpen = false;
	ChatPn->SetVisibility(ESlateVisibility::Collapsed);
	ChatPreviewPn->SetVisibility(ESlateVisibility::Visible);

	// clear children
	ChatScrollBox->ClearChildren();
	ChatPreviewScrollBox->ClearChildren();
}

void UChatUI::OnChatTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	UE_LOG(LogTemp, Warning, TEXT("ChatUI: OnChatTextCommitted called with text: %s"), *Text.ToString());
	if (CommitMethod == ETextCommit::OnEnter)
	{
		if (ChatSubsystem)
		{
			FString Message = Text.ToString();
			if (!Message.IsEmpty())
			{
				ChatSubsystem->SendChatMessageInRoom(Message);
			}
		}
		ChatInput->SetText(FText::GetEmpty());
		CloseChat();
	}
}

void UChatUI::HandleNewChatMessage(const FString& Sender, const FString& Message)
{
	if (ChatNodeClass)
	{
		UChatNode* MainNode = CreateWidget<UChatNode>(GetWorld(), ChatNodeClass);
		UChatNode* PreviewNode = CreateWidget<UChatNode>(GetWorld(), ChatNodeClass);

		MainNode->SetChatMessage(Sender, Message);
		PreviewNode->SetChatMessage(Sender, Message);

		ChatScrollBox->AddChild(MainNode);
		ChatScrollBox->ScrollToEnd();

		ChatPreviewScrollBox->AddChild(PreviewNode);
		ChatPreviewScrollBox->ScrollToEnd();

		FTimerHandle RemovePreviewTimer;
		FTimerDelegate Delegate;

		Delegate.BindLambda([PreviewNode]()
			{
				if (PreviewNode && PreviewNode->IsValidLowLevel())
				{
					PreviewNode->RemoveFromParent();
				}
			});

		GetWorld()->GetTimerManager().SetTimer(
			RemovePreviewTimer,
			Delegate,
			2.0f,
			false
		);
	}
}

void UChatUI::SetChatOpen(bool bOpen)
{
	bIsChatOpen = bOpen;

	ChatPn->SetVisibility(bOpen ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	ChatPreviewPn->SetVisibility(bOpen ? ESlateVisibility::Collapsed : ESlateVisibility::Visible);

	if (bOpen)
	{
		ChatInput->SetKeyboardFocus();
	}
}

void UChatUI::OpenChat()
{
	if (bIsChatOpen)
	{
		return;
	}
	SetChatOpen(true);
}

void UChatUI::CloseChat()
{
	if (!bIsChatOpen)
	{
		return;
	}
	SetChatOpen(false);

	if (APlayerController* PC = GetOwningPlayer())
	{
		SetUserFocus(PC);
	}
}