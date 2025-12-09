// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/PickupComponent.h"
#include "Components/InventoryComponent.h"
#include "Components/WeaponComponent.h"
#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Weapons/WeaponTypes.h"
#include "Weapons/WeaponBase.h"
#include "Weapons/WeaponKnifeBasic.h"
#include "Net/UnrealNetwork.h"
#include "InputActionValue.h"
#include "Camera/CameraComponent.h"
#include "Components/TimelineComponent.h"
#include "Components/InteractComponent.h"
#include "Components/HealthComponent.h"     
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Components/PostProcessComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/Material.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Perception/AISense_Sight.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Components/AudioComponent.h"
#include "BaseCharacter.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnHit);

UCLASS()
class FPSDEMO_API ABaseCharacter : public ACharacter
{
    GENERATED_BODY()

protected:
    UPickupComponent* PickupComponent;

    UInventoryComponent* InventoryComp;

    UInteractComponent* InteractComp;

    UWeaponComponent* WeaponComp;

	UHealthComponent* HealthComp;

    UPROPERTY()
	AController* LastHitByController = nullptr;
	bool bLastHitWasHeadshot = false;

    UPROPERTY()
    UWeaponData* LastDamageCauser = nullptr;

    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRepSpeedWalkCurrently, Category = "Data")
	float SpeedWalkCurrently = NORMAL_WALK_SPEED;
    UFUNCTION()
	void OnRepSpeedWalkCurrently();

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera")
    UCameraComponent* CurrentCamera;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* FirstPersonCamera;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* ThirdPersonCamera;

    USkeletalMeshComponent* mesh;
	USkeletalMeshComponent* MeshFps;

    UPROPERTY(EditDefaultsOnly, Category = "Crouch")
    UCurveFloat* CrouchCurve;   // assign in editor

    FTimeline CrouchTimeline;
    UFUNCTION()
    void HandleCrouchProgress(float Value);

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
    UAnimMontage* EquipMontage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
    UAnimMontage* ThrowNadeMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
	UAnimMontage* HoldNadeMontage;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
    UAnimMontage* KnifeAttack1Montage;
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Animation")
    UAnimMontage* KnifeAttack2Montage;

    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Viewmodel")
    UMaterial* MaterialOverlayBase;
    UMaterialInstanceDynamic* MaterialOverlayMID;

    USceneComponent* ThrowableLocation;

    // Lifecycle
    virtual void BeginPlay() override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    void StartRunning();
    void StopRunning();
    void CustomCrouch();
    void CustomUnCrouch();
    UFUNCTION(BlueprintCallable)
	void UpdateView();
    UFUNCTION(BlueprintPure)
    EWeaponTypes GetWeaponType();
    UFUNCTION(BlueprintPure)
	EWeaponSubTypes GetWeaponSubType();

    UFUNCTION(Server, Reliable)
    void ServerSetCrouching(bool bNewCrouching);

	UFUNCTION()
	void OnRep_Crouching();
    void UpdateAttachLocationWeapon();
    void DropWeapon();

    UFUNCTION(Server, Reliable)
    void ServerSetAiming(bool bNewAiming);

    UFUNCTION()
    void OnRep_IsAiming();

	virtual void UpdateAimingState();
	void OnMeleeNotify();

	UFUNCTION()
	void OnNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FX")
    UNiagaraSystem* BloodFx;

    UPROPERTY(EditDefaultsOnly)
    TSubclassOf<AActor> DeathCameraProxyClass; // BP with physics root + camera

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    UAnimMontage* FireRifleMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    UAnimMontage* FirePistolMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    UAnimMontage* ReloadMontage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    UAnimMontage* ReloadPistolMontage;

    float TargetFOV = 90.0f;

    UPROPERTY()
    UAIPerceptionStimuliSourceComponent* StimuliSource;

    void OnRep_PlayerState() override;

    bool bAppliedTeamMesh = false;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spike")
    USoundBase* PlantingSpikeSound;
    UPROPERTY()
    UAudioComponent* PlantSpikeAudioComp = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spike")
    USoundBase* DefusingSpikeSound;
    UPROPERTY()
	UAudioComponent* DefuseSpikeAudioComp = nullptr;
public:
    ABaseCharacter();

    UPROPERTY(BlueprintReadWrite, Category = "State")
    bool bCloseToWall = false;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bReloading = false;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bEquipped = false;

    UPROPERTY(BlueprintReadOnly, ReplicatedUsing = OnRep_Crouching, Category = "State")
    bool bCrouching = false;

    UPROPERTY(BlueprintReadWrite, Category = "State")
    bool bIsFPS = false;

    UPROPERTY(ReplicatedUsing = OnRep_IsAiming, BlueprintReadOnly, Category = "State")
    bool bAiming = false;

    UPROPERTY()
    bool bHoldingShift = false;


    UPROPERTY(BlueprintReadOnly, Category = "Data")
    FVector2D moveInput;
    UPROPERTY(BlueprintReadWrite, Category = "State")
    float AimSensitivity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    UAnimMontage* FireMeleeMontage;

    USplineComponent* ThrowSpline;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PostProcess")
    UMaterialParameterCollection* FlashCollection;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PostProcess")
    UCurveFloat* StunCurve;

    FTimeline StunTimeline;
	float BaseStunDuration = 0.f;

    USceneCaptureComponent2D* ViewmodelCapture;

    // Timeline callback
    UFUNCTION()
    void OnStunTimelineUpdate(float Value);

    // Optional: called when timeline finishes
    UFUNCTION()
    void OnStunTimelineFinished();


    virtual void Tick(float DeltaTime) override;
    static constexpr float MAX_WALK_SPEED = 600.f;
    static constexpr float NORMAL_WALK_SPEED = 400.f;
    static constexpr float MELEE_WALK_SPEED = 500.f;
    static constexpr float CROUCH_WALK_SPEED = 200.f;
	static constexpr float AIM_WALK_SPEED = 250.f;
    void AddWeapon(AWeaponBase* weapon);
    FORCEINLINE UPickupComponent* GetPickupComponent() const {
        return PickupComponent;
    }
    FORCEINLINE UInventoryComponent* GetInventoryComponent() const {
        return InventoryComp;
    }
    FORCEINLINE UWeaponComponent* GetWeaponComponent() const {
        return WeaponComp;
    }
    UFUNCTION()
    USkeletalMeshComponent* GetCurrentMesh();

    virtual void ClickAim();
    bool IsRunning();
	bool IsFpsViewMode() const { return bIsFPS; }
    void PlayEquipWeaponAnimation(EWeaponTypes WeaponType);
	float GetSpeedWalkCurrently();
    void SetSpeedWalkCurrently(float NewSpeed);
	void HandleUpdateSpeedWalkCurrently();
    virtual float TakeDamage(
        float DamageAmount,
        struct FDamageEvent const& DamageEvent,
        class AController* EventInstigator,
        class AActor* DamageCauser
    ) override;

	void PlayThrowNadeMontage();
	void PlayHoldNadeMontage();
	void PlayMontage(UAnimMontage* MontageToPlay);
	bool IsCloseToWall() const { return false; }
    FVector GetThrowableLocation() const {
        return ThrowableLocation->GetComponentLocation();
	}
	void PlayMeleeAttackAnimation(int32 AttackIndex);

    UPROPERTY(EditDefaultsOnly, Category = "Decal")
    UMaterialInterface* MeleeHitDecal;

	void HandleDeath();

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_HandleDeath();

    UFUNCTION(Server, Reliable) void ServerRevive();
    UFUNCTION(NetMulticast, Reliable) void Multicast_ReviveFX();

    virtual void PlayReloadMontage(UWeaponData* WeaponConf);

    UFUNCTION(Client, UnReliable)
    void ClientPlayHitEffect();

	FOnHit OnHit;

    void PlayBloodFx(const FVector& HitLocation);
	void PlayStunEffect(const float& Strength);

    void SetPosViewmodelCaptureForGun();
    FVector3d ViewmodelCaptureDefaultPos;
	FRotator ViewmodelCaptureDefaultRot;

    void PlayFireRifleMontage(FVector TargetPoint);
    void PlayFirePistolMontage(FVector TargetPoint);
    void StartAiming();
    void StopAiming();
    virtual void Jump() override;
    virtual void StopJumping() override;
    void ClickCrouch();
    void ChangeView();
    float GetAimSensitivity();

    UPROPERTY(EditAnywhere, Category = "AI")
    UBehaviorTree* BehaviorTree;

    void SetMeshBaseOnTeam();
	void PlayPlantSpikeEffect();
	void StopPlantSpikeEffect();
    void PlayDefuseSpikeEffect();
	void StopDefuseSpikeEffect();
};
