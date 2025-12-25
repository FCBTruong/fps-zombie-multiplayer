// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/TimelineComponent.h"
#include "Items/ItemIds.h"
#include "Types/EquippedAnimState.h"
#include "BaseCharacter.generated.h"

class UPickupComponent;
class UInventoryComponent;
class UInteractComponent;
class UWeaponComponent;
class UHealthComponent;
class UCameraComponent;
class UEquipComponent;
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
class UCharAudioComponent;
class UCharCameraComponent;
class UActionStateComponent;
class UItemVisualComponent;
class UWeaponFireComponent;
class UWeaponMeleeComponent;
class UThrowableComponent;
class UCharacterAsset;
class UPostProcessComponent;


DECLARE_MULTICAST_DELEGATE(FOnHit);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnAimingChanged, bool);

UENUM(BlueprintType)
enum class EMovementState : uint8
{
    Normal,
	Slow
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
    virtual void OnStartCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;
    virtual void OnEndCrouch(float HalfHeightAdjust, float ScaledHalfHeightAdjust) override;

protected:
	// Components
    UPROPERTY(VisibleAnywhere, Category = "Components")
    TObjectPtr<UWeaponComponent> WeaponComp;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    TObjectPtr<UPickupComponent> PickupComponent;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    TObjectPtr<UInventoryComponent> InventoryComp;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    TObjectPtr<UInteractComponent> InteractComp;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    TObjectPtr<UHealthComponent> HealthComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UAnimationComponent> AnimationComp;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    TObjectPtr<UCharAudioComponent> AudioComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UCharCameraComponent> CameraComp;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    TObjectPtr<UEquipComponent> EquipComp;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    TObjectPtr<UItemVisualComponent> ItemVisualComp;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    TObjectPtr<UActionStateComponent> ActionStateComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UWeaponFireComponent> WeaponFireComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UWeaponMeleeComponent> WeaponMeleeComp;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    TObjectPtr<UThrowableComponent> ThrowableComp;

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

    UPROPERTY(EditDefaultsOnly, Category = "Init|FX")
    TObjectPtr<UNiagaraSystem> BloodFx;

    UPROPERTY(EditDefaultsOnly, Category =  "Init|Camera")
    TSubclassOf<AActor> DeathCameraProxyClass;

    UPROPERTY(EditDefaultsOnly, Category = "Init|Decal")
    TObjectPtr<UMaterialInterface> MeleeHitDecal;

    UPROPERTY(EditDefaultsOnly, Category = "Init|AI")
    TObjectPtr<UBehaviorTree> BehaviorTree;

    UPROPERTY(EditDefaultsOnly, Category = "Init|Crouch")
    TObjectPtr<UCurveFloat> CrouchCurve = nullptr;

    UPROPERTY(Transient)
    TObjectPtr<UCharacterAsset> CachedCharacterAsset;

    UPROPERTY(VisibleAnywhere)
    UPostProcessComponent* FlashPP = nullptr;
protected:
    // ===== Runtime State =====
    bool bLastHitWasHeadshot;
    bool bAppliedTeamMesh;
    bool bHasBeginPlayRun;
    float LastFootstepTime;
    float BaseStunDuration;
    float BasePivotFpsZ = 0.f;
    float CurrentCrouchCompZ = 0.f;
    float CrouchFromZ = 0.f;
    float CrouchToZ = 0.f;

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
    float AimSensitivity = 1.0f;
    UPROPERTY(ReplicatedUsing = OnRep_IsAiming)
    bool bIsAiming = false;
    UPROPERTY(ReplicatedUsing = OnRep_CurrentMovementState)
    EMovementState CurrentMovementState = EMovementState::Normal;

protected:
    // ===== Timelines =====
	FTimeline StunTimeline;
    FTimeline CrouchTimeline;
protected:
    // ===== Internal Functions =====
    void ApplyAimingVisuals();
    void OnMeleeNotify();
    void PlayFootstepSound();
    void UpdateFootstepSound(float DeltaTime);
    void PlayLandingSound();
    void ApplyTeamMesh();
    void HandleDeath();
	bool CanPlayFootstep() const;
	bool IsBot() const;
    void UpdateCurrentWeapon(EItemId CurrentWeaponId);

    UFUNCTION(BlueprintPure)
    EEquippedAnimState GetEquippedAnimState() const;

    UFUNCTION()
    void HandleCrouchProgress(float Alpha);

    UFUNCTION(BlueprintPure)
    EWeaponTypes GetWeaponType() const;
    UFUNCTION(BlueprintPure)
	EWeaponSubTypes GetWeaponSubType() const;

    // ===== Networking RPC =====
    UFUNCTION(Server, Reliable)
    void ServerSetAiming(bool bNewAiming);
    UFUNCTION(Server, Reliable) 
    void ServerRevive();
    UFUNCTION(Server, Reliable)
    void ServerSetIsSlow(bool bNewIsSlow); 

    // ===== Networking Multicast =====
    UFUNCTION(NetMulticast, Unreliable)
    void MulticastReviveFX();
    UFUNCTION(NetMulticast, Reliable)
    void MulticastHandleDeath();

	// ===== Networking OnRep =====
    UFUNCTION()
    void OnRep_IsAiming();
    UFUNCTION()
    void OnRep_CurrentMovementState();

	// ===== Client RPC =====
    UFUNCTION(Client, Reliable)
    void ClientPlayHitEffect();

	// ===== Other UFUNCTIONS =====
    UFUNCTION()
    void OnStunTimelineFinished();
    UFUNCTION()
    void OnStunTimelineUpdate(float Value);
    UFUNCTION()
    void OnNotifyBegin(FName NotifyName, const FBranchingPointNotifyPayload& BranchingPointPayload);

  
public:
    // ===== Public API =====
    void RequestStartAiming();
	void RequestStopAiming();
	void UpdateMaxWalkSpeed();
    void PlayBloodFx(const FVector& HitLocation);
	void PlayStunEffect(const float& Strength);
    void StartAiming();
    void StopAiming();   
    void ChangeView();
	void OnPlantSpikeStarted();
	void OnPlantSpikeStopped();
    void OnDefuseSpikeStarted();
	void OnDefuseSpikeStopped();
    void ApplyRotationMode(bool bIsPlayer);
    void RequestCrouch();
    void RequestUnCrouch();
    void RequestSlowMovement(bool bEnable);
	void RequestJump();
    float GetSpeedWalkRatio() const;
    float GetAimSensitivity() const;
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
	UEquipComponent* GetEquipComponent() const;
	UWeaponFireComponent* GetWeaponFireComponent() const;
	UWeaponMeleeComponent* GetWeaponMeleeComponent() const;
	UThrowableComponent* GetThrowableComponent() const; 
	UActionStateComponent* GetActionStateComponent() const;
	UCharAudioComponent* GetAudioComponent() const;

    UFUNCTION(BlueprintCallable)
    EMovementState GetCurrentMovementState() const;

	// ===== Delegates =====
    FOnHit OnHit;
    FOnAimingChanged OnAimingChanged;

	// ===== Constants =====
    static constexpr float MAX_WALK_SPEED = 600.f;
    static constexpr float NORMAL_WALK_SPEED = 400.f;
    static constexpr float CROUCH_WALK_SPEED = 200.f;
    static constexpr float SLOW_WALK_SPEED = 180.f;
    static constexpr float FOOTSTEP_SPEED_MIN = 300.f;
};
