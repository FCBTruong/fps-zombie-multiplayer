// Fill out your copyright notice in the Description page of Project Settings.


#include "Chat/ChatNode.h"

void UChatNode::SetChatMessage(const FString& Sender, const FString& Message)
{
	if (SenderLb)
	{
		SenderLb->SetText(FText::FromString(Sender));
	}
	if (MessageLb)
	{
		MessageLb->SetText(FText::FromString(Message));
	}
}