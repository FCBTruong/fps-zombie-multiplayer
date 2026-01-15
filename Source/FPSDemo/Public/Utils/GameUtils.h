// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class FPSDEMO_API GameUtils
{
public:
	GameUtils();
	~GameUtils();

	static FString PointNumber(int32 Number);
	static FString GenerateMd5Token();
};
