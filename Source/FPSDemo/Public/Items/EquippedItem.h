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

protected:
	bool bIsFpsView;
	
	UPROPERTY(Transient)
	TWeakObjectPtr<USceneCaptureComponent2D> ViewmodelCapture;

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
public:	
	virtual void InitFromConfig(UItemConfig* InConfig);

	UMeshComponent* GetMainMesh() const;

	virtual void SetViewFps(bool bIsFps);
	virtual void SetViewCapture(USceneCaptureComponent2D* InCapture);
};
