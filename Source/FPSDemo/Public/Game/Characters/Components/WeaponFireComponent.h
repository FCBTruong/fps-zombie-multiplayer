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
	bool TraceShot(const AActor* IgnoredActor, const FVector& Start, const FVector& Dir,
		FHitResult& OutHit, FVector& OutEnd, double ShotTime) const;
	bool CanReload() const;
	int32 ComputeShotIndex(float NowServerTime) const;
	FVector ComputeShotDirDeterministic(const FVector& AimDir, float NowServerTime, int32 Seed) const;

	// RPC
	UFUNCTION(Server, Reliable)
	void ServerStartFire(int InBurstSeed, double ShotTime);

	UFUNCTION(Server, Reliable)
	void ServerStopFire();

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastFireFX(FVector_NetQuantize TargetPoint);

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

	// Smooth recoil state
	FVector2D RecoilCurrent = FVector2D::ZeroVector; // applied this frame (Pitch, Yaw)
	FVector2D RecoilTarget = FVector2D::ZeroVector;  // where recoil wants to go

	UPROPERTY(Transient)
	TObjectPtr<const UFirearmConfig> CurrentFirearmConfig = nullptr;
};
