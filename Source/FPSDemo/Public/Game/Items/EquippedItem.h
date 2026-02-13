// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EquippedItem.generated.h"

class USkeletalMeshComponent;
class UStaticMeshComponent;
class UItemConfig;
class USceneCaptureComponent2D;

enum class EActiveMesh : uint8 { None, Skeletal, Static };
UCLASS()
class FPSDEMO_API AEquippedItem : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AEquippedItem();

	virtual void InitFromConfig(UItemConfig* InConfig);
	const UItemConfig* GetItemConfig() const { return Config; }
	UMeshComponent* GetMainMesh() const;
	virtual void SetViewFps(bool bIsFps);

protected:
	bool bIsFpsView;

	UPROPERTY()
	TObjectPtr<UItemConfig> Config;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> Root;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USkeletalMeshComponent> WeaponMesh;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> WeaponStaticMesh;

	EActiveMesh ActiveMesh = EActiveMesh::None;

	virtual void SetActiveMeshSkeletal(USkeletalMesh* InMesh);
	virtual void SetActiveMeshStatic(UStaticMesh* InMesh);
	virtual void ApplyConfig();
};
