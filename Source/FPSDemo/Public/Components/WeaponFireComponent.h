// WeaponFireComponent.h

#pragma once

#include "CoreMinimal.h"
#include "Components/RoleGatedComponent.h"
#include "Net/UnrealNetwork.h"
#include "Items/ItemIds.h"
#include "WeaponFireComponent.generated.h"

class ABaseCharacter;
class UEquipComponent;
class UInventoryComponent;
class UActionStateComponent;
class UItemVisualComponent;
class UCharAudioComponent;
class UFirearmConfig;

enum EFireEnableReason
{
	OK,
	NoAmmo,
	Undefined,
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class FPSDEMO_API UWeaponFireComponent : public URoleGatedComponent
{
	GENERATED_BODY()

public:
	UWeaponFireComponent();

	void Initialize(
		UInventoryComponent* InInventory,
		UActionStateComponent* InAction,
		UItemVisualComponent* InVisual
	);

	// Input-facing
	void RequestStartFire();
	void RequestStopFire();
	void RequestReload();
	void RequestFireOnce();

	bool CanWeaponAim() const;
	UFUNCTION()
	void OnActiveItemChanged(EItemId NewId);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnEnabledChanged(bool bNowEnabled) override;
private:
	// Server auth
	void StartFire_ServerAuth();
	void StopFire_ServerAuth();
	void FireOnce_ServerAuth();
	float GetTotalSpreadDeg(float NowServerTime) const;
	float GetAirSpreadDeg() const;
	float GetMovementSpreadDeg() const;
	float GetMoveAlphaForSpread() const;
	bool CanReload() const;
	void HandleReload();
	void OnFinishedReload();
	void ApplyRecoilLocal();

#if !UE_SERVER
	// Owning client prediction (visuals only)
	void FireOnce_PredictedLocal();
#endif

	EFireEnableReason CanFireNow() const;
	bool IsOwningClient() const;

	void GetAim(FVector& OutStart, FVector& OutDir) const;
	bool TraceShot(const AActor* IgnoredActor, const FVector& Start, const FVector& Dir,
		FHitResult& OutHit, FVector& OutEnd) const;

	float GetServerTimeSeconds() const;
	int32 ComputeShotIndex(float NowServerTime) const;
	FVector ComputeShotDirDeterministic(const FVector& AimDir, float NowServerTime, int32 Seed) const;
	void UpdateBurstSpreadOnShot(float NowServerTime);

	// RPC
	UFUNCTION(Server, Reliable)
	void ServerStartFire(int InBurstSeed);

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
	UPROPERTY(Transient) TObjectPtr<UInventoryComponent> InventoryComp = nullptr;
	UPROPERTY(Transient) TObjectPtr<UActionStateComponent> ActionStateComp = nullptr;
	UPROPERTY(Transient) TObjectPtr<UItemVisualComponent> VisualComp = nullptr;

	UPROPERTY(Transient) TObjectPtr<UCharAudioComponent> AudioComp = nullptr;

	UPROPERTY(Transient) TObjectPtr<ABaseCharacter> Character = nullptr;

	// Timers
	FTimerHandle FireTimer_Server;
	FTimerHandle ReloadTimer;

#if !UE_SERVER
	FTimerHandle FireTimer_Local;
#endif

	// Replicated determinism inputs
	UPROPERTY(Replicated)
	float FireStartTimeServer = 0.f;

	int32 BurstSeed = 0;

	// Local spread state (used for both predicted + server firing)
	float BurstAccDeg = 0.f;
	float LastShotTime = 0.f;

	// Tunables
	float BurstResetDelay = 0.25f;
	int ShotCount = 0;

	UPROPERTY(Transient)
	TObjectPtr<const UFirearmConfig> CurrentFirearmConfig = nullptr;
};
