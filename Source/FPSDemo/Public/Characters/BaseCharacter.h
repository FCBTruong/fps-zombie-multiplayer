// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/TimelineComponent.h"
#include "BaseCharacter.generated.h"

class UPickupComponent;
class UInventoryComponent;
class UInteractComponent;
class UWeaponComponent;
class UHealthComponent;
class UCameraComponent;
class USpringArmComponent;
class USceneCaptureComponent2D;
class USkeletalMeshComponent;
class USceneComponent;
class UCurveFloat;
class UAnimMontage;
class UNiagaraSystem;
class UAIPerceptionStimuliSourceComponent;
class USoundBase;
class UMaterial;
class UMaterialInstanceDynamic;
class UMaterialParameterCollection;
class UBehaviorTree;
class UTextureRenderTarget2D;
class AWeaponBase;
class UWeaponData;
class UAudioComponent;
class USplineComponent;
class UAnimationComponent;


DECLARE_MULTICAST_DELEGATE(FOnHit);

UENUM(BlueprintType)
enum class EMovementState : uint8
{
    Normal,
	Slow,
	Crouch
};


USTRUCT(BlueprintType)
struct FCharacterSoundSet
{
    GENERATED_BODY()

    UPROPERTY(EditDefaultsOnly, Category = "Sound") TObjectPtr<USoundBase> PlantingSpike = nullptr;
    UPROPERTY(EditDefaultsOnly, Category = "Sound") TObjectPtr<USoundBase> DefusingSpike = nullptr;
    UPROPERTY(EditDefaultsOnly, Category = "Sound") TObjectPtr<USoundBase> Landing = nullptr;
    UPROPERTY(EditDefaultsOnly, Category = "Sound") TObjectPtr<USoundBase> Footstep = nullptr;
};

UCLASS()
class FPSDEMO_API ABaseCharacter : public ACharacter
{
    GENERATED_BODY()

public:
	ABaseCharacter();

protected:
    //  ===== Lifecycle =====
    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    virtual void OnRep_PlayerState() override;
    virtual void Landed(const FHitResult& Hit) override;
    virtual void Tick(float DeltaTime) override;
    virtual float TakeDamage(
        float DamageAmount,
        struct FDamageEvent const& DamageEvent,
        class AController* EventInstigator,
        class AActor* DamageCauser
    ) override;
    virtual void Jump() override;
    virtual void StopJumping() override;
    virtual void Destroyed() override;
    virtual void BecomeViewTarget(APlayerController* PC) override;
    virtual void EndViewTarget(APlayerController* PC) override;
    virtual void PossessedBy(AController* NewController) override;
    virtual void OnRep_Controller() override;

protected:
	// Components
    UPROPERTY(VisibleAnywhere, Category = "Components")
    TObjectPtr<UPickupComponent> PickupComponent;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    TObjectPtr<UInventoryComponent> InventoryComp;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    TObjectPtr<UInteractComponent> InteractComp;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    TObjectPtr<UWeaponComponent> WeaponComp;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    TObjectPtr<UHealthComponent> HealthComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UAnimationComponent> AnimationComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USceneComponent> FpsPivot;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    TObjectPtr<UCameraComponent> CameraFps;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    TObjectPtr<USkeletalMeshComponent> MeshFps;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    TObjectPtr<USceneCaptureComponent2D> ViewmodelCap;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    TObjectPtr<UCameraComponent> CameraTps;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    TObjectPtr<USpringArmComponent> CameraBoom;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    TObjectPtr<UAIPerceptionStimuliSourceComponent> StimuliSource;

	// ===== Init Data =====

    UPROPERTY(EditDefaultsOnly, Category = "Init|Crouch")
    TObjectPtr<UCurveFloat> CrouchCurve;

    UPROPERTY(EditDefaultsOnly, Category = "Init|Viewmodel")
    TObjectPtr<UMaterial> MaterialOverlayBase;

    UPROPERTY(EditDefaultsOnly, Category = "Init|FX")
    TObjectPtr<UNiagaraSystem> BloodFx;

    UPROPERTY(EditDefaultsOnly, Category = "Init|Sound")
    FCharacterSoundSet Sounds;

    UPROPERTY(EditDefaultsOnly, Category =  "Init|Camera")
    TSubclassOf<AActor> DeathCameraProxyClass;

    UPROPERTY(EditDefaultsOnly, Category = "Init|Flash")
    TObjectPtr<UMaterialParameterCollection> FlashCollection;

    UPROPERTY(EditDefaultsOnly, Category = "Init|Flash")
    TObjectPtr<UCurveFloat> StunCurve;

    UPROPERTY(EditDefaultsOnly, Category = "Init|Decal")
    TObjectPtr<UMaterialInterface> MeleeHitDecal;

    UPROPERTY(EditDefaultsOnly, Category = "Init|AI")
    TObjectPtr<UBehaviorTree> BehaviorTree;


protected:
    // ===== Runtime State =====
    bool bLastHitWasHeadshot;
    bool bAppliedTeamMesh;
    bool bHasBeginPlayRun;
    bool bRecallBVT_AtBegin;
    bool bIsFPS;
    bool bHoldingShift;
    bool bIsBot;
    float LastFootstepTime;
    float TargetFOV;
    float BaseStunDuration;

    UPROPERTY()
    TObjectPtr<UAudioComponent> DefuseSpikeAudioComp;

    UPROPERTY(Transient)
    TObjectPtr<UAudioComponent> PlantSpikeAudioComp;

    UPROPERTY(Transient)
    TObjectPtr<UMaterialInstanceDynamic> MaterialOverlayMID;

    UPROPERTY()
    TWeakObjectPtr<AController> LastHitByController;

    UPROPERTY()
    TWeakObjectPtr<UWeaponData> LastDamageCauser;

    UPROPERTY(Transient)
    TObjectPtr<UTextureRenderTarget2D> ViewmodelRenderTarget;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    TObjectPtr<USceneComponent> ThrowableLocation;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    TObjectPtr<USplineComponent> ThrowSpline;

protected:
	// ===== Replicated Properties =====
	UPROPERTY(Replicated)
    float AimSensitivity = 1.0f;
    UPROPERTY(ReplicatedUsing = OnRep_IsAiming)
    bool bAiming = false;
    UPROPERTY(ReplicatedUsing = OnRep_IsCrouching)
    bool bIsCrouching = false;
    UPROPERTY(ReplicatedUsing = OnRep_CurrentMovementState)
    EMovementState CurrentMovementState = EMovementState::Normal;

protected:
    // ===== Timelines =====
    FTimeline CrouchTimeline;
	FTimeline StunTimeline;

protected:
    // ===== Internal Functions =====
    void StartRunning();
    void StopRunning();
    void CustomCrouch();
    void CustomUnCrouch();
    void UpdateAimingState();
    void OnMeleeNotify();
    void PlayFootstepSound();
    void UpdateFootstepSound(float DeltaTime);
    void PlayLandingSound();
    void UpdateAttachLocationWeapon();
    void DropWeapon();
    void ApplyTeamMesh();
    void SetFpsView(bool bNewIsFPS);

    UFUNCTION(BlueprintPure)
    EWeaponTypes GetWeaponType() const;
    UFUNCTION(BlueprintPure)
	EWeaponSubTypes GetWeaponSubType() const;

    // ===== Networking RPC =====
    UFUNCTION(Server, Unreliable)
    void ServerSetAiming(bool bNewAiming);
    UFUNCTION(Server, Reliable) 
    void ServerRevive();
    UFUNCTION(Server, Unreliable)
    void ServerSetIsSlow(bool bNewIsSlow); 
    UFUNCTION(Server, Unreliable)
    void ServerSetCrouching(bool bNewCrouching);

    // ===== Networking Multicast =====
    UFUNCTION(NetMulticast, Unreliable)
    void MulticastReviveFX();
    UFUNCTION(NetMulticast, Unreliable)
    void MulticastHandleDeath();

	// ===== Networking OnRep =====
    UFUNCTION()
    void OnRep_IsAiming();
    UFUNCTION()
    void OnRep_CurrentMovementState();
    UFUNCTION()
    void OnRep_IsCrouching();
	UFUNCTION()
	void OnNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload);

	// ===== Client RPC =====
    UFUNCTION(Client, Unreliable)
    void ClientPlayHitEffect();

	// ===== Other UFUNCTIONS =====
    UFUNCTION()
    void HandleCrouchProgress(float Value);
    UFUNCTION()
    void OnStunTimelineFinished();
    UFUNCTION()
    void OnStunTimelineUpdate(float Value);
  
public:
    // ===== Public API =====
    void ClickAim();
	void HandleUpdateSpeedWalkCurrently();
	void HandleDeath();
    void PlayBloodFx(const FVector& HitLocation);
	void PlayStunEffect(const float& Strength);
    void SetPosViewmodelCaptureForGun();
    void StartAiming();
    void StopAiming();   
    void ChangeView();
	void PlayPlantSpikeEffect();
	void StopPlantSpikeEffect();
    void PlayDefuseSpikeEffect();
	void StopDefuseSpikeEffect();
    void UpdateView();
    void ApplyRotationMode(bool bIsPlayer);
    void RequestCrouch();
    void RequestUnCrouch();
    void RequestSlowMovement(bool bEnable);
	void RequestJump();
    float GetSpeedWalkRatio();
    float GetAimSensitivity();
    bool IsAlive() const;
    bool IsFpsViewMode() const;
    FVector GetThrowableLocation() const;
    USceneCaptureComponent2D* GetViewmodelCapture() const;
    UBehaviorTree* GetBehaviorTree() const;
	USplineComponent* GetThrowSpline() const;
    UPickupComponent* GetPickupComponent() const;
    UInventoryComponent* GetInventoryComponent() const;
	UHealthComponent* GetHealthComponent() const;
	UInteractComponent* GetInteractComponent() const;
	UAnimationComponent* GetAnimationComponent() const;
    UWeaponComponent* GetWeaponComponent() const;
    USkeletalMeshComponent* GetCurrentMesh() const;

    UFUNCTION(BlueprintCallable)
    EMovementState GetCurrentMovementState() const;

	// ===== Delegates =====
    FOnHit OnHit;

	// ===== Constants =====
    static constexpr float MAX_WALK_SPEED = 600.f;
    static constexpr float NORMAL_WALK_SPEED = 400.f;
    static constexpr float MELEE_WALK_SPEED = 500.f;
    static constexpr float CROUCH_WALK_SPEED = 200.f;
    static constexpr float AIM_WALK_SPEED = 250.f;
    static constexpr float SLOW_WALK_SPEED = 200.f;
    static constexpr float DEFAULT_FPS_FOV = 103.f;
};
