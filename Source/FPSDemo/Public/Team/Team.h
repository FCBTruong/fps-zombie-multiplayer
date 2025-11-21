// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

/**
 * 
 */
class FPSDEMO_API Team
{

private:
	bool bIsEliminated = false;
public:
	Team();
	~Team();

	bool IsEliminated() const { return bIsEliminated; }
};
