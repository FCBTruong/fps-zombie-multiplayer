// Fill out your copyright notice in the Description page of Project Settings.

#pragma once


#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/TimelineComponent.h"
#include "Items/ItemIds.h"
#include "Types/EquippedAnimState.h"
#include "Characters/CharacterRole.h"
#include "BaseCharacter.generated.h"

class UPickupComponent;
class UInventoryComponent;
class UInteractComponent;
class UHealthComponent;
class UCameraComponent;
class USpikeComponent;
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
class UTextureRenderTarget2D;
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
class UItemConfig;
class URoleComponent;

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
	TObjectPtr<USpikeComponent> SpikeComp;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    TObjectPtr<UItemVisualComponent> ItemVisualComp;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    TObjectPtr<UActionStateComponent> ActionStateComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UWeaponFireComponent> WeaponFireComp;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	TObjectPtr<UWeaponMeleeComponent> WeaponMeleeComp;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    TObjectPtr<URoleComponent> RoleComp;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    TObjectPtr<UThrowableComponent> ThrowableComp;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<USceneComponent> FpsPivot;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components")
    TObjectPtr<UCameraComponent> CameraFps;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    TObjectPtr<USkeletalMeshComponent> MeshFps;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    TObjectPtr<UCameraComponent> CameraTps;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    TObjectPtr<USpringArmComponent> CameraBoom;

    UPROPERTY(VisibleAnywhere, Category = "Components")
    TObjectPtr<UAIPerceptionStimuliSourceComponent> StimuliSource;

	// ===== Init Data =====

    UPROPERTY(EditDefaultsOnly, Category = "Init|FX")
    TObjectPtr<UNiagaraSystem> BloodFx;

    UPROPERTY(EditDefaultsOnly, Category = "Init|Decal")
    TObjectPtr<UMaterialInterface> MeleeHitDecal;

    UPROPERTY(EditDefaultsOnly, Category = "Init|Crouch")
    TObjectPtr<UCurveFloat> CrouchCurve = nullptr;

    UPROPERTY(Transient)
    TObjectPtr<UCharacterAsset> CachedCharacterAsset;

    UPROPERTY(VisibleAnywhere)
    UPostProcessComponent* FlashPP = nullptr;
protected:
    // ===== Runtime State =====
    bool bLastHitWasHeadshot;
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

    UPROPERTY()
    TWeakObjectPtr<AController> LastHitByController;

    UPROPERTY()
    TWeakObjectPtr<const UItemConfig> LastDamageCauser;

protected:
	// ===== Replicated Properties =====
    float AimSensitivity = 1.0f;
    UPROPERTY(ReplicatedUsing = OnRep_IsAiming)
    bool bIsAiming = false;
    UPROPERTY(ReplicatedUsing = OnRep_CurrentMovementState)
    EMovementState CurrentMovementState = EMovementState::Normal;
    UPROPERTY(ReplicatedUsing = OnRep_SpeedMultiplier)
    float SpeedMultiplier = 1.0f;
protected:
    // ===== Timelines =====
	FTimeline StunTimeline;
    FTimeline CrouchTimeline;
    FTimerHandle HitSlowTimer;
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
    void HandleRoleChanged(ECharacterRole OldRole, ECharacterRole NewRole);
    void ApplyVisualByRole(ECharacterRole NewRole);
    void ApplyInputByRole(ECharacterRole NewRole);
	void ApplyLoadoutByRole(ECharacterRole NewRole);
    void ApplyHitSlow(float Multiplier, float Duration);
    void ClearHitSlow();
	void BecomeHero_Internal();

    UFUNCTION(BlueprintPure)
    EEquippedAnimState GetEquippedAnimState() const;

    UFUNCTION()
    void HandleCrouchProgress(float Alpha);

    // ===== Networking RPC =====
    UFUNCTION(Server, Reliable)
    void ServerSetAiming(bool bNewAiming);
    UFUNCTION(Server, Reliable) 
    void ServerRevive();
    UFUNCTION(Server, Reliable)
    void ServerSetIsSlow(bool bNewIsSlow); 
	UFUNCTION(Server, Reliable)
	void ServerBecomeHero();

    // ===== Networking Multicast =====
    UFUNCTION(NetMulticast, Unreliable)
    void MulticastReviveFX();
    UFUNCTION(NetMulticast, Reliable)
    void MulticastPlayerDeath();

	// ===== Networking OnRep =====
    UFUNCTION()
    void OnRep_IsAiming();
    UFUNCTION()
    void OnRep_CurrentMovementState();
    UFUNCTION()
    void OnRep_SpeedMultiplier();

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
    virtual void RequestStartAiming();
    virtual void RequestStopAiming();
    virtual void UpdateMaxWalkSpeed();
    virtual void PlayBloodFx(const FVector& HitLocation, const FVector& HitNormal);
    virtual void PlayStunEffect(const float& Strength);
    virtual void ChangeView();
    virtual void OnPlantSpikeStarted();
    virtual void OnPlantSpikeStopped();
    virtual void OnDefuseSpikeStarted();
    virtual void OnDefuseSpikeStopped();
    virtual void ApplyRotationMode();
    virtual void RequestCrouch();
    virtual void RequestUnCrouch();
    virtual void RequestSlowMovement(bool bEnable);
    virtual void RequestJump();
    virtual float GetSpeedWalkRatio() const;
    virtual float GetAimSensitivity() const;
    virtual bool IsAlive() const;
    virtual bool IsDead() const;
    virtual bool IsFpsViewMode() const;
    virtual bool IsAiming() const;
    virtual bool IsCharacterRole(ECharacterRole InRole) const;
    virtual void ZombieAttack();
    virtual void RequestBecomeHero();
	ECharacterRole GetCharacterRole() const;
    FVector GetThrowableLocation() const;
    UPickupComponent* GetPickupComponent() const;
    UInventoryComponent* GetInventoryComponent() const;
	UHealthComponent* GetHealthComponent() const;
	UInteractComponent* GetInteractComponent() const;
	UAnimationComponent* GetAnimationComponent() const;
    USkeletalMeshComponent* GetCurrentMesh() const;
	UEquipComponent* GetEquipComponent() const;
	UWeaponFireComponent* GetWeaponFireComponent() const;
	UWeaponMeleeComponent* GetWeaponMeleeComponent() const;
	UThrowableComponent* GetThrowableComponent() const; 
	USpikeComponent* GetSpikeComponent() const;
	UActionStateComponent* GetActionStateComponent() const;
	UCharAudioComponent* GetAudioComponent() const;
	URoleComponent* GetRoleComponent() const;

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
