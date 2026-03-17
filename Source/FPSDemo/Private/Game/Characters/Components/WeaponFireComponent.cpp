// WeaponFireComponent.cpp

#include "Game/Characters/Components/WeaponFireComponent.h"
#include "Game/Characters/BaseCharacter.h"
#include "Game/Characters/Components/EquipComponent.h"
#include "Game/Characters/Components/InventoryComponent.h"
#include "Game/Characters/Components/ItemVisualComponent.h"
#include "Game/Characters/Components/ActionStateComponent.h" // adjust include to your path
#include "GameFramework/GameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "Game/Utils/Damage/MyDamageType.h"
#include "Game/Utils/Damage/MyPointDamageEvent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Shared/Data/Items/FirearmConfig.h"
#include "Game/Characters/Components/AnimationComponent.h"
#include "Game/Characters/Components/CharAudioComponent.h"
#include "Shared/System/ItemsManager.h"
#include "Game/Utils/Damage/DamageHelpers.h"
#include "Game/GameManager.h"
#include "Game/Characters/Components/LagCompensationComponent.h"
#include "EngineUtils.h"

UWeaponFireComponent::UWeaponFireComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(true);
}

void UWeaponFireComponent::Init()
{;
	Character = Cast<ABaseCharacter>(GetOwner());
	check(Character);

	InventoryComp = Character->GetInventoryComponent();
	ActionStateComp = Character->GetActionStateComponent();
	VisualComp = Character->GetItemVisualComponent();
	LagCompensationComp = Character->GetLagCompensationComponent();
	AudioComp = Character->GetAudioComponent();
	AnimComp = Character->GetAnimationComponent();

	check(InventoryComp);
	check(ActionStateComp);
	check(VisualComp);
	check(LagCompensationComp);
	check(AudioComp);
	check(AnimComp);
}

void UWeaponFireComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (CurrentFirearmConfig && BurstAccDeg > 0.f)
	{
		const bool bIsFiring = ActionStateComp->IsInState(EActionState::Firing);

		if (!bIsFiring)
		{
			const float RecoverRate = CurrentFirearmConfig->BurstRecoverDegPerSec; // deg/sec
			BurstAccDeg = FMath::Max(0.f, BurstAccDeg - RecoverRate * DeltaTime);
		}
	}

#if !UE_SERVER
	if (!IsOwningClient()) return;

	// Tune these
	const float RecoilApplySpeed = 20.0f;   // how fast camera moves to recoil target

	// Current smoothly follows target
	const FVector2D Prev = RecoilCurrent;
	RecoilCurrent = FMath::Vector2DInterpTo(RecoilCurrent, RecoilTarget, DeltaTime, RecoilApplySpeed);

	// Apply only the frame delta
	const FVector2D Delta = RecoilCurrent - Prev;

	Character->AddControllerPitchInput(Delta.X);
	Character->AddControllerYawInput(Delta.Y);
#endif
}

void UWeaponFireComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
#if !UE_SERVER
		World->GetTimerManager().ClearTimer(FireTimer_Local);
#endif
		World->GetTimerManager().ClearTimer(FireTimer_Server);
		World->GetTimerManager().ClearTimer(ReloadTimer);
	}

	Super::EndPlay(EndPlayReason);
}

bool UWeaponFireComponent::IsOwningClient() const
{
	return Character->IsLocallyControlled();
}

EFireEnableReason UWeaponFireComponent::CanFireNow() const
{
	if (!IsEnabled())
	{
		return EFireEnableReason::Undefined;
	}
	if (!CurrentFirearmConfig) {
		return EFireEnableReason::Undefined;
	}
	if (!Character->IsAlive())
	{
		return EFireEnableReason::Undefined;
	}
	if (!ActionStateComp->CanFireNow())
	{
		return EFireEnableReason::Undefined;
	}

	float TimeSinceLastShot = GetServerTimeSeconds() - LastShotTime;
	if (TimeSinceLastShot < CurrentFirearmConfig->FireInterval * 0.8) // allow some leeway for timer inaccuracies
		return EFireEnableReason::Undefined;

	// Ammo check (only firearms should pass)
	const FWeaponState* State = InventoryComp->GetWeaponStateByItemId(CurrentFirearmConfig->Id);
	if (!State)
		return EFireEnableReason::Undefined;
	if (State->AmmoInClip <= 0)
		return EFireEnableReason::NoAmmo;

	return EFireEnableReason::OK;
}

void UWeaponFireComponent::OnActiveItemChanged(EItemId NewId)
{
#if !UE_SERVER
	GetWorld()->GetTimerManager().ClearTimer(FireTimer_Local);
#endif

	GetWorld()->GetTimerManager().ClearTimer(FireTimer_Server);
	GetWorld()->GetTimerManager().ClearTimer(ReloadTimer);

	if (NewId == EItemId::NONE)
	{
		CurrentFirearmConfig = nullptr;
		return;
	}

	UItemsManager* ItemsManager = UItemsManager::Get(GetWorld());
	CurrentFirearmConfig = Cast<UFirearmConfig>(ItemsManager->GetItemById(NewId));

	BurstAccDeg = 0.f;
	LastShotTime = 0.f;
}

void UWeaponFireComponent::RequestStartFire()
{
	if (!IsEnabled())
	{
		return;
	}
	if (!CurrentFirearmConfig) {
		return;
	}

	EFireEnableReason Reason = CanFireNow();
	if (Reason != EFireEnableReason::OK)
	{
		// if reason is bullet ammo empty, could trigger a "dry fire" sound/animation here
		if (Reason == EFireEnableReason::NoAmmo) {
			AudioComp->PlaySound3D(CurrentFirearmConfig->DryFireSound);
		}
		return;
	}

	BurstSeed = FMath::Rand();
#if !UE_SERVER
	// Client prediction (visual only)
	if (!Character->HasAuthority() && IsOwningClient())
	{
		// should reset spread state on start
		ShotCount = 0;
		FireOnce_PredictedLocal();
		GetWorld()->GetTimerManager().SetTimer(
			FireTimer_Local,
			this,
			&UWeaponFireComponent::FireOnce_PredictedLocal,
			CurrentFirearmConfig->FireInterval,
			true
		);
	}
#endif
	// if is aiming, stop aim
	if (Character->IsAiming()) {
		Character->RequestStopAiming();
	}
	if (Character->HasAuthority())
	{
		StartFire_ServerAuth();
	}
	else
	{
		double ServerTime = GetServerTimeSeconds();
		ServerStartFire(BurstSeed, ServerTime);
	}
}

void UWeaponFireComponent::RequestStopFire()
{
#if !UE_SERVER
	if (!Character->HasAuthority() && IsOwningClient())
	{
		GetWorld()->GetTimerManager().ClearTimer(FireTimer_Local);
	}
#endif

	if (Character->HasAuthority())
	{
		StopFire_ServerAuth();
	}
	else
	{
		ServerStopFire();
	}
}

void UWeaponFireComponent::ServerStartFire_Implementation(int InBurstSeed, double ShotTime)
{
	if (!IsEnabled())
		return;
	BurstSeed = InBurstSeed;
	ClientShotTimeOffset = GetServerTimeSeconds() - ShotTime;
	StartFire_ServerAuth();
}

void UWeaponFireComponent::ServerStopFire_Implementation()
{
	StopFire_ServerAuth();
}

void UWeaponFireComponent::StartFire_ServerAuth()
{
	if (!IsEnabled())
	{
		return;
	}
	if (!Character->HasAuthority())
	{
		return;
	}
	if (!CurrentFirearmConfig) {
		return;
	}

	if (ActionStateComp->IsInState(EActionState::Firing)) // already firing
	{
		return;
	}
	if (CanFireNow() != EFireEnableReason::OK)
	{
		return;
	}
	if (!ActionStateComp->TrySetState(EActionState::Firing))
	{
		return;
	}

	ShotCount = 0;
	// First shot immediately, then loop
	FireOnce_ServerAuth();

	GetWorld()->GetTimerManager().SetTimer(
		FireTimer_Server,
		this,
		&UWeaponFireComponent::FireOnce_ServerAuth,
		CurrentFirearmConfig->FireInterval,
		true,
		CurrentFirearmConfig->FireInterval
	);
}

void UWeaponFireComponent::StopFire_ServerAuth()
{
	if (!Character->HasAuthority())
	{
		return;
	}
	GetWorld()->GetTimerManager().ClearTimer(FireTimer_Server);
	if (ActionStateComp->IsInState(EActionState::Firing)) {
		ActionStateComp->TrySetState(EActionState::Idle);
	}
}

#if !UE_SERVER
void UWeaponFireComponent::FireOnce_PredictedLocal()
{
	if (CanFireNow() != EFireEnableReason::OK)
	{
		GetWorld()->GetTimerManager().ClearTimer(FireTimer_Local);
		return;
	}
	ShotCount++;
	const float Now = GetServerTimeSeconds();
	FVector Start, AimDir;
	GetAim(Start, AimDir);
	const FVector ShotDir = ComputeShotDirDeterministic(AimDir, Now, BurstSeed);
	UpdateBurstSpreadOnShot(Now);

	TArray<FPenetrationHitResult> Hits;
	TArray<FBulletImpactData> Impacts;
	FVector ShotEnd;
	TraceShot(Character, Start, ShotDir, Hits, Impacts, ShotEnd, Now, false);

	if (VisualComp)
	{
		VisualComp->PlayFireFX(Impacts, ShotEnd);
	}

	LastShotTime = Now;
	ApplyRecoilLocal(); // camera jitter
}
#endif

void UWeaponFireComponent::FireOnce_ServerAuth()
{
	if (!Character->HasAuthority()) {
		return;
	}
	if (CanFireNow() != EFireEnableReason::OK)
	{
		StopFire_ServerAuth();
		return;
	}

	ShotCount++;
	const float Now = GetServerTimeSeconds();
	FVector Start, AimDir;
	GetAim(Start, AimDir);
	const FVector ShotDir = ComputeShotDirDeterministic(AimDir, Now, BurstSeed);
	UpdateBurstSpreadOnShot(Now);

	TArray<FPenetrationHitResult> PenHits;
	TArray<FBulletImpactData> Impacts;
	FVector ShotEnd;
	double ShotTimestamp = Now - ClientShotTimeOffset;
	TraceShot(Character, Start, ShotDir, PenHits, Impacts, ShotEnd, ShotTimestamp);
	// Consume ammo on server
	InventoryComp->ConsumeAmmo(CurrentFirearmConfig->Id, 1);

	for (const FPenetrationHitResult& PenHit : PenHits)
	{
		FDamageApplyParams Params;
		Params.BaseDamage = PenHit.Damage;
		Params.WeaponId = CurrentFirearmConfig->Id;
		Params.DamageTypeClass = UMyDamageType::StaticClass();
		Params.bEnableHeadshot = true;
		Params.bIsPenetrationHit = PenHit.bIsPenetrationHit;
		Params.Hit = PenHit.Hit;

		DamageHelpers::ApplyMyPointDamage(
			Params,
			Character->GetController(),
			nullptr
		);
	}

	MulticastFireFX(Impacts, ShotEnd);
	LastShotTime = Now;

#if !UE_SERVER
	if (IsOwningClient()) // listen server host
	{
		VisualComp->PlayFireFX(Impacts, ShotEnd);
		// ApplyRecoilLocal();
	}
#endif
}

void UWeaponFireComponent::MulticastFireFX_Implementation(const TArray<FBulletImpactData>& Impacts, FVector_NetQuantize ShotEnd)
{
	if (IsOwningClient())
	{
		return;
	}

	VisualComp->PlayFireFX(Impacts, ShotEnd);
}

void UWeaponFireComponent::GetAim(FVector& OutStart, FVector& OutDir) const
{
	FRotator ViewRot;
	Character->GetActorEyesViewPoint(OutStart, ViewRot);
	OutDir = ViewRot.Vector();
}

void UWeaponFireComponent::TraceShot(
	const AActor* IgnoredActor,
	FVector Start,
	const FVector& Dir,
	TArray<FPenetrationHitResult>& OutHits,
	TArray<FBulletImpactData>& OutImpacts,
	FVector& OutEnd,
	double ShotTime,
	bool bIsRewindTrace
) const
{
	constexpr float MaxDistance = 10000.f;
	constexpr float SurfaceExitStep = 4.f;
	constexpr float StartOffset = 1.f;
	float Damage = CurrentFirearmConfig->Damage;
	float DamageLossPerPenetration = CurrentFirearmConfig->DamageLossPerPenetration;
	float MaxPenetrationDepth = CurrentFirearmConfig->MaxPenetrationDepth;
	float PenetrationPerCent = 0.5f;
	
	OutHits.Empty();
	OutImpacts.Empty();
	const FVector FullTraceEnd = Start + Dir * 10000.f;
	OutEnd = FullTraceEnd;

	const FVector ShotDir = Dir.GetSafeNormal();
	const FVector FinalEnd = Start + ShotDir * MaxDistance;
	OutEnd = FinalEnd;

	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_WorldStatic);
	FCollisionQueryParams Params(SCENE_QUERY_STAT(WeaponTrace), false);

	UWorld* World = GetWorld();
	if (!World) return;

	bool bDidPenetrate = false;
	while (Damage > 0)
	{
		// 1. Find world hit
		FHitResult WorldHit;

		// only care about world objects for penetration, characters will be handled separately with lag compensation
		const bool bHitWorld = World->LineTraceSingleByObjectType(
			WorldHit,
			Start,
			FinalEnd,
			ObjectParams,
			Params
		);

		if (bHitWorld)
		{
			FBulletImpactData EntryImpact;
			EntryImpact.ImpactPoint = WorldHit.ImpactPoint;
			EntryImpact.ImpactNormal = WorldHit.ImpactNormal;
			EntryImpact.Kind = EBulletImpactKind::Entry;
			OutImpacts.Add(EntryImpact);

			FPenetrationHitResult PenHit;
			PenHit.Hit = WorldHit;
			PenHit.Damage = Damage;

			OutHits.Add(PenHit);
			OutEnd = WorldHit.ImpactPoint;
		}
		else {
			OutEnd = FinalEnd;
		}

		// check characters before this wall with lag compensation
		for (TActorIterator<ABaseCharacter> It(GetWorld()); It; ++It)
		{
			ABaseCharacter* TargetChar = *It;
			if (!TargetChar) continue;
			if (TargetChar == Character) continue;
			if (IgnoredActor && TargetChar == IgnoredActor) continue;
			if (!TargetChar->IsAlive()) continue;

			FHitResult CharacterHit;
			bool bConfirmed = false;
			if (bIsRewindTrace)
			{
				ULagCompensationComponent* LagComp = TargetChar->GetLagCompensationComponent();
				if (!LagComp) continue;

				bConfirmed = LagComp->ConfirmHitRewind(
					Character,
					Start,
					OutEnd,
					ShotTime,
					CharacterHit
				);
			}
			else
			{
				if (USkeletalMeshComponent* MeshComp = TargetChar->GetMesh())
				{
					bConfirmed = MeshComp->LineTraceComponent(
						CharacterHit,
						Start,
						OutEnd,
						Params
					);
				}
			}

			if (!bConfirmed) continue;

			FPenetrationHitResult PenHit;
			PenHit.Hit = CharacterHit;
			PenHit.Damage = Damage;
			PenHit.bIsPenetrationHit = bDidPenetrate;
			OutHits.Add(PenHit);

			FBulletImpactData EntryImpact;
			EntryImpact.ImpactPoint = CharacterHit.ImpactPoint;
			EntryImpact.ImpactNormal = CharacterHit.ImpactNormal;
			EntryImpact.Kind = EBulletImpactKind::Entry;
			EntryImpact.ImpactActor = TargetChar;
			OutImpacts.Add(EntryImpact);
		}

		// Find exit point for this world object
		if (bHitWorld)
		{
			FVector ExitPos = FVector::ZeroVector;
			bool bFoundExit = false;

			for (float Depth = SurfaceExitStep; Depth <= MaxPenetrationDepth; Depth += SurfaceExitStep)
			{
				const FVector Probe = WorldHit.ImpactPoint + ShotDir * Depth;

				if (!World->LineTraceTestByObjectType(
					Probe,
					Probe + ShotDir * StartOffset,
					ObjectParams,
					Params
				))
				{
					ExitPos = Probe;
					bFoundExit = true;
					break;
				}
			}

			if (!bFoundExit)
			{
				break;
			}

			FBulletImpactData ExitImpact;
			ExitImpact.ImpactPoint = ExitPos;
			ExitImpact.ImpactNormal = -ShotDir;
			ExitImpact.Kind = EBulletImpactKind::Exit;
			OutImpacts.Add(ExitImpact);

			const float Thickness = FVector::Distance(WorldHit.ImpactPoint, ExitPos);
			Damage -= Thickness * PenetrationPerCent;
			UE_LOG(LogTemp, Log, TEXT("Trace Shot: Thickness=%.1f, Damage=%.1f"), Thickness, Damage);

			if (Damage <= 0.f)
			{
				break;
			}

			Start = ExitPos + ShotDir * StartOffset;
			bDidPenetrate = true;
		}
		else {
			break;
		}
	}
}

float UWeaponFireComponent::GetServerTimeSeconds() const
{
	const AGameStateBase* GS = GetWorld()->GetGameState();
	return GS ? GS->GetServerWorldTimeSeconds() : GetWorld()->GetTimeSeconds();
}

int32 UWeaponFireComponent::ComputeShotIndex(float NowServerTime) const
{
	return ShotCount;
}

FVector UWeaponFireComponent::ComputeShotDirDeterministic(
	const FVector& AimDir,
	float NowServerTime,
	int32 Seed
) const
{
	const int32 ShotIndex = ComputeShotIndex(NowServerTime);
	const int32 PerShotSeed = Seed ^ (ShotIndex * 196613);
	FRandomStream Stream(PerShotSeed);
	const float SpreadDeg = GetTotalSpreadDeg(NowServerTime);
	float SpreadRad = FMath::DegreesToRadians(SpreadDeg);

	// if crouching, reduce spread by half
	if (Character->bIsCrouched) {
		SpreadRad *= 0.5f;
	}
	return Stream.VRandCone(AimDir, SpreadRad, SpreadRad).GetSafeNormal();
}

float UWeaponFireComponent::GetTotalSpreadDeg(float NowServerTime) const
{
	// NOTE: BurstAccDeg should already be updated at the moment of firing on both sides.
	float MoveSpreadDeg = GetMovementSpreadDeg();
	const float Total = CurrentFirearmConfig->BaseDeg + MoveSpreadDeg + GetAirSpreadDeg() + BurstAccDeg;
	return FMath::Min(Total, CurrentFirearmConfig->MaxTotalDeg);
}

float UWeaponFireComponent::GetMovementSpreadDeg() const
{
	float x = CurrentFirearmConfig->MoveAddDeg * GetMoveAlphaForSpread();
	return CurrentFirearmConfig->MoveAddDeg * GetMoveAlphaForSpread();
}

float UWeaponFireComponent::GetAirSpreadDeg() const
{
	const UCharacterMovementComponent* Move = Character->GetCharacterMovement();
	return Move->IsFalling() ? CurrentFirearmConfig->AirAddDeg : 0.0f;
}

float UWeaponFireComponent::GetMoveAlphaForSpread() const
{
	const float Speed2D = Character->GetVelocity().Size2D();
	const float Alpha = FMath::Clamp(Speed2D / 500, 0.0f, 1.0f); // 500 is max walk speed

	// Apply curve
	const float Exp = FMath::Max(0.01f, CurrentFirearmConfig->MoveCurveExp);
	return FMath::Pow(Alpha, Exp);
}

void UWeaponFireComponent::UpdateBurstSpreadOnShot(float NowServerTime)
{
	// Add per-shot burst spread
	BurstAccDeg = FMath::Min(BurstAccDeg + CurrentFirearmConfig->PerShotAddDeg, CurrentFirearmConfig->MaxBurstAddDeg);
	LastShotTime = NowServerTime;
}

void UWeaponFireComponent::RequestReload() {
	if (!IsEnabled()) {
		return;
	}
	if (!Character->IsAlive()) {
		return;
	}
	if (!CanReload()) {
		return;
	}

	if (Character->HasAuthority()) {
		HandleReload();
	}
	else
	{
		ServerReload();
	}
}

void UWeaponFireComponent::ServerReload_Implementation()
{
	if (!IsEnabled()) {
		return;
	}
	HandleReload();
}

bool UWeaponFireComponent::CanReload() const {
	if (!IsEnabled()) {
		return false;
	}
	if (!Character->IsAlive()) {
		return false;
	}
	if (!CurrentFirearmConfig) {
		return false;
	}
	// check action state
	if (!ActionStateComp->CanReloadNow()) {
		return false;
	}

	// check ammo availability
	const FWeaponState* State = InventoryComp->GetWeaponStateByItemId(CurrentFirearmConfig->Id);
	if (!State) {
		return false;
	}
	if (State->AmmoInClip >= State->MaxAmmoInClip) {
		return false;
	}
	if (State->AmmoReserve <= 0) {
		return false;
	}

	return true;
}

// Server function
void UWeaponFireComponent::HandleReload()
{
	if (!CanReload()) {
		return;
	}
	if (ActionStateComp->IsInState(EActionState::Firing)) {
		StopFire_ServerAuth();
	}
	if (!ActionStateComp->TrySetState(EActionState::Reloading))
	{
		return;
	}

	MulticastReload();
	GetWorld()->GetTimerManager().SetTimer(
		ReloadTimer,
		this,
		&UWeaponFireComponent::HandleFinishedReload,
		2.5f,
		false
	);
}

void UWeaponFireComponent::MulticastReload_Implementation()
{
	if (!CurrentFirearmConfig) {
		return;
	}

	if (CurrentFirearmConfig->FirearmType == EFirearmType::Pistol)
	{
		AnimComp->PlayReloadPistolMontage();
	}
	else {
		AnimComp->PlayReloadRifleMontage();
	}

	AudioComp->PlaySound3D(CurrentFirearmConfig->ReloadMagSound);
}

// Server function
void UWeaponFireComponent::HandleFinishedReload()
{
	if (!GetOwner()->HasAuthority()) {
		return;
	}
	if (!Character->IsAlive()) {
		return;
	}
	if (!CurrentFirearmConfig) {
		return;
	}
	if (!ActionStateComp->IsInState(EActionState::Reloading)) {
		return;
	}
	ActionStateComp->TrySetState(EActionState::Idle);
	InventoryComp->ReloadWeapon(CurrentFirearmConfig->Id);
	OnFinishedReload.Broadcast();
}

bool UWeaponFireComponent::CanWeaponAim() const {
	if (!IsEnabled()) {
		return false;
	}

	if (!Character->IsAlive()) {
		return false;
	}
	if (!CurrentFirearmConfig) {
		return false;
	}

	return CurrentFirearmConfig->bHasScopeEquipped;
}

void UWeaponFireComponent::ApplyRecoilLocal()
{
#if !UE_SERVER
	if (!IsOwningClient()) {
		return;
	}

	const float PitchKick =
		CurrentFirearmConfig->RecoilPitchPerShot +
		FMath::FRandRange(-CurrentFirearmConfig->RecoilPitchJitter, CurrentFirearmConfig->RecoilPitchJitter);

	const float YawKick =
		FMath::FRandRange(-CurrentFirearmConfig->RecoilYawPerShot, CurrentFirearmConfig->RecoilYawPerShot);

	// Add recoil into target (negative pitch = camera goes up)
	RecoilTarget.X += -PitchKick;
	RecoilTarget.Y += YawKick;
#endif
}

void UWeaponFireComponent::OnEnabledChanged(bool bNowEnabled)
{
	if (UWorld* World = GetWorld())
	{
#if !UE_SERVER
		World->GetTimerManager().ClearTimer(FireTimer_Local);
#endif
		World->GetTimerManager().ClearTimer(FireTimer_Server);
		World->GetTimerManager().ClearTimer(ReloadTimer);
	}

	if (!bNowEnabled)
	{
		ShotCount = 0;
		BurstAccDeg = 0.f;
		LastShotTime = 0.f;
	}
}

bool UWeaponFireComponent::IsFiring() const
{
	return ActionStateComp->IsInState(EActionState::Firing);
}