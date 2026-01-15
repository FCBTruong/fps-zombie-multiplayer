// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LoginUI.generated.h"

/**
 * 
 */
UCLASS()
class FPSDEMO_API ULoginUI : public UUserWidget
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;
private:
	void TryLogin();
};
