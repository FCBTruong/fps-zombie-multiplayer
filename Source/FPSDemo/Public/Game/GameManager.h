// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Game/Items/Pickup/PickupData.h"
#include "Network/DedicatedServerClient.h"
#include "Game/Data/MatchInfo.h"
#include "GameManager.generated.h"

class UGlobalDataAsset;
class UCharacterAsset;
class ABaseCharacter;

struct FProcessParameters;
/**
 * 
 */
UCLASS()
class FPSDEMO_API UGameManager : public UGameInstance
{
	GENERATED_BODY()

private:
	void InitGameLift();
	void ShutdownGameLiftServer(bool bSuccess);
	TSharedPtr<FProcessParameters> ProcessParameters;
protected:
	virtual void Init() override;
	virtual void OnStart() override;
public:
	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UGlobalDataAsset> GlobalData = nullptr;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UCharacterAsset> CharacterAsset = nullptr;

	
	static UGameManager* Get(UObject* WorldContextObject);
	TUniquePtr<DedicatedServerClient> DsClient;

	void StartMatch(FMatchInfo MatchInfo);
	void InitServerConfig(
		const FString& InRoomId,
		const FString& InToken);
	void RequestMatchDataAndStart();
	void SetCurrentMatchInfo(const FMatchInfo& NewMatchInfo) { CurrentMatchInfo = NewMatchInfo; }
	const FMatchInfo& GetCurrentMatchInfo() const { return CurrentMatchInfo; }
private:
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void CreateHostSession();
	FName PendingMapName;
	FString PendingOptions;
	FDelegateHandle OnCreateHandle;
	FMatchInfo CurrentMatchInfo;
};
