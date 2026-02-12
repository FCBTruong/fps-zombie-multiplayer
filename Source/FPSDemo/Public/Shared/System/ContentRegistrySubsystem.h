// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Shared/Data/AvatarData.h"
#include "ContentRegistrySubsystem.generated.h"

/**
 * 
 */
UCLASS()
class FPSDEMO_API UContentRegistrySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    /** Returns avatar texture by AvatarId (returns nullptr if not found) */
    UTexture2D* GetAvatarTextureById(const FString& AvatarId) const;

private:
    /** AvatarId -> AvatarData */
    UPROPERTY()
    TMap<FString, TObjectPtr<UAvatarData>> AvatarDataMap;
};
