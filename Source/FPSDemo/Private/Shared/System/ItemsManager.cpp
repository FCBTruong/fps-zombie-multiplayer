// Fill out your copyright notice in the Description page of Project Settings.


#include "Shared/System/ItemsManager.h"
#include "Shared/Data/Items/ItemConfig.h"
#include "AssetRegistry/AssetRegistryModule.h"

const UItemConfig* UItemsManager::GetItemById(EItemId Id) const
{
    if (ItemConfigMap.Contains(Id)) {
        return ItemConfigMap[Id];
    }
    return nullptr;
}

void UItemsManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);  

    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    TArray<FAssetData> AssetList;
    AssetRegistryModule.Get().GetAssetsByPath(FName("/Game/Main/Data/Items"), AssetList, true);

    for (const FAssetData& Asset : AssetList)
    {
        if (UItemConfig* Data = Cast<UItemConfig>(Asset.GetAsset()))
        {
            ItemList.Add(Data);
            ItemConfigMap.Add(Data->Id, Data);
        }
    }
}

UItemsManager* UItemsManager::Get(UWorld* WorldContextObject)
{
    if (!WorldContextObject)
    {
        return nullptr;
    }

    UGameInstance* GameInstance = WorldContextObject->GetGameInstance();
    if (!GameInstance)
    {
        return nullptr;
    }

    return GameInstance->GetSubsystem<UItemsManager>();
}