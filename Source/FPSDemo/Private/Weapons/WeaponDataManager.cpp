// WeaponDataManager.cpp
#include "Weapons/WeaponDataManager.h"
#include "AssetRegistry/AssetRegistryModule.h"
UWeaponData* UWeaponDataManager::GetWeaponById(FName Id) const
{
	for (UWeaponData* Data : WeaponList)
	{
		if (Data && Data->Id == Id)
			return Data;
	}
	return nullptr;
}


void UWeaponDataManager::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

   
    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

    TArray<FAssetData> AssetList;
    AssetRegistryModule.Get().GetAssetsByPath(FName("/Game/Main/Data/Weapons"), AssetList, true);

    for (const FAssetData& Asset : AssetList)
    {
        if (Asset.AssetClassPath == UWeaponData::StaticClass()->GetClassPathName())
        {
            if (UWeaponData* Data = Cast<UWeaponData>(Asset.GetAsset()))
            {
                WeaponList.Add(Data);
            }
        }
    }


    UE_LOG(LogTemp, Log, TEXT("WeaponDataManager loaded %d weapon assets"), WeaponList.Num());
}