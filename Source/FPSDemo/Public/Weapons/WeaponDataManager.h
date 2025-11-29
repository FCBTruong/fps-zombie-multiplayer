// WeaponDataManager.h
#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "WeaponData.h"
#include "WeaponDataManager.generated.h"

UCLASS()
class FPSDEMO_API UWeaponDataManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

private:
	TArray<UWeaponData*> WeaponList;
	TMap<EItemId, UWeaponData*> WeaponDataMap;
public:
	UWeaponData* GetWeaponById(EItemId Id) const;

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	const TArray<UWeaponData*>& GetAllWeapons() const { return WeaponList; }
};
