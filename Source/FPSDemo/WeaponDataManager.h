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

public:
	UPROPERTY(EditAnywhere, Category = "Weapons")
	TArray<UWeaponData*> WeaponList;

	UWeaponData* GetWeaponById(FName Id) const;

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
};
