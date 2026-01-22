// Fill out your copyright notice in the Description page of Project Settings.


#include "ContentRegistrySubsystem.h"
#include "Engine/AssetManager.h"
#include "Engine/Texture2D.h"

void UContentRegistrySubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    UAssetManager& AM = UAssetManager::Get();

    // Use FPrimaryAssetType, not a raw string
    TArray<FPrimaryAssetId> AssetIds;
    AM.GetPrimaryAssetIdList(FPrimaryAssetType(TEXT("AvatarData")), AssetIds);

    UE_LOG(LogTemp, Log, TEXT("UContentRegistrySubsystem: Found %d AvatarData assets"), AssetIds.Num());

    for (const FPrimaryAssetId& AssetId : AssetIds)
    {
        TSharedPtr<FStreamableHandle> Handle = UAssetManager::Get().LoadPrimaryAsset(AssetId);
        if (Handle.IsValid())
        {
            Handle->WaitUntilComplete();
        }

        UAvatarData* Data = Cast<UAvatarData>(UAssetManager::Get().GetPrimaryAssetObject(AssetId));
        if (!Data)
        {
            UE_LOG(LogTemp, Warning, TEXT("Failed to load %s"), *AssetId.ToString());
            continue;
        }

        AvatarDataMap.Add(Data->AvatarId, Data);
    }
}
UTexture2D* UContentRegistrySubsystem::GetAvatarTextureById(const FString& AvatarId) const
{
    const TObjectPtr<UAvatarData>* Found = AvatarDataMap.Find(AvatarId);
    if (!Found || !(*Found))
    {
        return nullptr;
    }

    return (*Found)->Texture.LoadSynchronous();
}