// WeaponFireComponent.h

#pragma once

#include "CoreMinimal.h"
#include "Game/Characters/Components/RoleGatedComponent.h"
#include "Net/UnrealNetwork.h"
#include "Shared/Types/ItemId.h"
#include "WeaponFireComponent.generated.h"

class ABaseCharacter;
class UEquipComponent;
class UInventoryComponent;
class UActionStateComponent;
class UItemVisualComponent;
class UCharAudioComponent;
class UFirearmConfig;
class ULagCompensationComponent;
class UAnimationComponent;

USTRUCT()
struct FPenetrationHitResult
{
	GENERATED_BODY()

	FHitResult Hit;
	float Damage = 0.f;
	bool bIsPenetrationHit = false;
};

UENUM()
enum class EBulletImpactKind : uint8
{
	Entry,
	Exit
};

USTRUCT()
struct FBulletImpactData
{
	GENERATED_BODY()

	UPROPERTY()
	FVector_NetQuantize ImpactPoint = FVector::ZeroVector;

	UPROPERTY()
	FVector_NetQuantizeNormal ImpactNormal = FVector::UpVector;

	UPROPERTY()
	EBulletImpactKind Kind = EBulletImpactKind::Entry;

	UPROPERTY()
	TObjectPtr<AActor> ImpactActor = nullptr;
};

enum EFireEnableReason
{
	OK,
	NoAmmo,
	Undefined,
};

DECLARE_MULTICAST_DELEGATE(FOnFinishedReload);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class FPSDEMO_API UWeaponFireComponent : public URoleGatedComponent
{
	GENERATED_BODY()

public:
	UWeaponFireComponent();
	void Init();

	// Input-facing
	void RequestStartFire();
	void RequestStopFire();
	void RequestReload();

	bool CanWeaponAim() const;
	bool IsFiring() const;
	UFUNCTION()
	void OnActiveItemChanged(EItemId NewId);
	EFireEnableReason CanFireNow() const;

	// Delegates
	FOnFinishedReload OnFinishedReload;
protected:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void OnEnabledChanged(bool bNowEnabled) override;
private:
	// Server auth
	void StartFire_ServerAuth();
	void StopFire_ServerAuth();
	void FireOnce_ServerAuth();
	void HandleReload();
	void HandleFinishedReload();
	void ApplyRecoilLocal();
	void GetAim(FVector& OutStart, FVector& OutDir) const;
	void UpdateBurstSpreadOnShot(float NowServerTime);

	float GetTotalSpreadDeg(float NowServerTime) const;
	float GetAirSpreadDeg() const;
	float GetMovementSpreadDeg() const;
	float GetMoveAlphaForSpread() const;
	float GetServerTimeSeconds() const;

#if !UE_SERVER
	// Owning client prediction (visuals only)
	void FireOnce_PredictedLocal();
#endif
	bool IsOwningClient() const;
	void TraceShot(const AActor* IgnoredActor, FVector Start, const FVector& Dir,
		TArray<FPenetrationHitResult>& OutHits, TArray<FBulletImpactData>& OutImpacts, 
		FVector& OutEnd, double ShotTime, bool bIsRewind = true) const;
	bool CanReload() const;
	int32 ComputeShotIndex(float NowServerTime) const;
	FVector ComputeShotDirDeterministic(const FVector& AimDir, float NowServerTime, int32 Seed) const;

	// RPC
	UFUNCTION(Server, Reliable)
	void ServerStartFire(int InBurstSeed, double ShotTime);

	UFUNCTION(Server, Reliable)
	void ServerStopFire();

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastFireFX(const TArray<FBulletImpactData>& Impacts, FVector_NetQuantize ShotEnd);

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastReload();
	UFUNCTION(Server, Reliable)
	void ServerReload();
private:
	// Dependencies
	UPROPERTY(Transient) 
	TObjectPtr<UInventoryComponent> InventoryComp = nullptr;

	UPROPERTY(Transient) 
	TObjectPtr<UActionStateComponent> ActionStateComp = nullptr;

	UPROPERTY(Transient) 
	TObjectPtr<UItemVisualComponent> VisualComp = nullptr;

	UPROPERTY(Transient) 
	TObjectPtr<UCharAudioComponent> AudioComp = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<UAnimationComponent> AnimComp = nullptr;

	UPROPERTY(Transient)
	TObjectPtr<ULagCompensationComponent> LagCompensationComp = nullptr;

	UPROPERTY(Transient) 
	TObjectPtr<ABaseCharacter> Character = nullptr;

	// Timers
	FTimerHandle FireTimer_Server;
	FTimerHandle ReloadTimer;

#if !UE_SERVER
	FTimerHandle FireTimer_Local;
#endif

	int32 BurstSeed = 0;
	double ClientShotTimeOffset = 0.0;

	// Local spread state (used for both predicted + server firing)
	float BurstAccDeg = 0.f;
	float LastShotTime = 0.f;

	// Tunables
	float BurstResetDelay = 0.25f;
	int ShotCount = 0;

	UPROPERTY(Transient)
	TObjectPtr<const UFirearmConfig> CurrentFirearmConfig = nullptr;

	float RecoilPitchCurrent = 0.0f;
	float RecoilPitchTarget = 0.0f;

	float RecoilPitchMaxOffset = -4.0f;     // clamp max upward offset
	float RecoilApplySpeed = 25.0f;
	float RecoilReturnSpeed = 10.0f;
};
