
#include "Game/Characters/Components/LagCompensationComponent.h"
#include "Game/Characters/BaseCharacter.h"           
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/GameStateBase.h"
#include "PhysicsEngine/PhysicsAsset.h"
#include "PhysicsEngine/SkeletalBodySetup.h"
#include "PhysicsEngine/BodyInstance.h"

ULagCompensationComponent::ULagCompensationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	SetIsReplicatedByDefault(false); // Server-side system
}

void ULagCompensationComponent::Init()
{
	OwnerCharacter = Cast<ABaseCharacter>(GetOwner());
	check(OwnerCharacter);
}

void ULagCompensationComponent::TickComponent(
	float DeltaTime,
	ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Only the server records history
	if (!OwnerCharacter->HasAuthority())
	{
		return;
	}
	SaveFrame();
}

double ULagCompensationComponent::GetServerTimeSeconds() const
{
	const AGameStateBase* GS = GetWorld()->GetGameState<AGameStateBase>();
	return GS ? GS->GetServerWorldTimeSeconds() : static_cast<double>(GetWorld()->GetTimeSeconds());
}

void ULagCompensationComponent::SaveFrame()
{
	USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh();
	if (!Mesh) return;

	UPhysicsAsset* PhysAsset = Mesh->GetPhysicsAsset();
	if (!PhysAsset) return;

	FRewindFrame NewFrame;
	NewFrame.Time = GetServerTimeSeconds();
	NewFrame.Bodies.Reserve(PhysAsset->SkeletalBodySetups.Num());

	for (const USkeletalBodySetup* BodySetup : PhysAsset->SkeletalBodySetups)
	{
		if (!BodySetup) continue;

		const FName BodyName = BodySetup->BoneName;
		FBodyInstance* BI = Mesh->GetBodyInstance(BodyName);
		if (!BI) continue;

		FRewindPhysBodyState State;
		State.BodyName = BodyName;
		State.WorldTransform = BI->GetUnrealWorldTransform();

		NewFrame.Bodies.Add(State);
	}

	if (NewFrame.Bodies.Num() == 0)
	{
		return;
	}

	// Avoid duplicate timestamps if Tick gets called multiple times with same time
	if (FrameHistory.Num() > 0 && FMath::IsNearlyEqual(FrameHistory[0].Time, NewFrame.Time, 1e-6))
	{
		FrameHistory[0] = MoveTemp(NewFrame);
	}
	else
	{
		FrameHistory.Insert(MoveTemp(NewFrame), 0); // Newest first
	}

	// Prune old frames
	while (FrameHistory.Num() > 1)
	{
		const double Newest = FrameHistory[0].Time;
		const double Oldest = FrameHistory.Last().Time;
		if ((Newest - Oldest) <= static_cast<double>(MaxRecordTime))
		{
			break;
		}
		FrameHistory.RemoveAt(FrameHistory.Num() - 1);
	}
}

void ULagCompensationComponent::CacheCurrentBodies(FRewindFrame& OutFrame) const
{
	OutFrame = FRewindFrame{};
	OutFrame.Time = GetServerTimeSeconds();

	USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh();
	if (!Mesh) return;

	UPhysicsAsset* PhysAsset = Mesh->GetPhysicsAsset();
	if (!PhysAsset) return;

	OutFrame.Bodies.Reserve(PhysAsset->SkeletalBodySetups.Num());

	for (const USkeletalBodySetup* BodySetup : PhysAsset->SkeletalBodySetups)
	{
		if (!BodySetup) continue;

		const FName BodyName = BodySetup->BoneName;
		FBodyInstance* BI = Mesh->GetBodyInstance(BodyName);
		if (!BI) continue;

		FRewindPhysBodyState State;
		State.BodyName = BodyName;
		State.WorldTransform = BI->GetUnrealWorldTransform();

		OutFrame.Bodies.Add(State);
	}
}

void ULagCompensationComponent::MoveBodiesToFrame(const FRewindFrame& Frame) const
{
	USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh();
	if (!Mesh) return;

	for (const FRewindPhysBodyState& State : Frame.Bodies)
	{
		FBodyInstance* BI = Mesh->GetBodyInstance(State.BodyName);
		if (!BI) continue;

		BI->SetBodyTransform(State.WorldTransform, ETeleportType::TeleportPhysics);
	}
}

void ULagCompensationComponent::RestoreBodies(const FRewindFrame& CachedFrame) const
{
	MoveBodiesToFrame(CachedFrame);
}

bool ULagCompensationComponent::GetInterpolatedFrameAtTime(double ShotTime, FRewindFrame& OutFrame) const
{
	OutFrame = FRewindFrame{};

	if (FrameHistory.Num() == 0)
	{
		return false;
	}

	// Newest frame is [0], oldest is Last()
	const FRewindFrame& Newest = FrameHistory[0];
	const FRewindFrame& Oldest = FrameHistory.Last();

	// Clamp to available history to keep function safe
	const double ClampedShotTime = FMath::Clamp(ShotTime, Oldest.Time, Newest.Time);

	// Direct edge cases
	if (ClampedShotTime >= Newest.Time)
	{
		OutFrame = Newest;
		return true;
	}
	if (ClampedShotTime <= Oldest.Time)
	{
		OutFrame = Oldest;
		return true;
	}

	// Find the pair [Younger, Older] around the shot time
	for (int32 i = 0; i < FrameHistory.Num() - 1; ++i)
	{
		const FRewindFrame& Younger = FrameHistory[i];     // newer
		const FRewindFrame& Older = FrameHistory[i + 1]; // older

		if (ClampedShotTime <= Younger.Time && ClampedShotTime >= Older.Time)
		{
			const double Den = Younger.Time - Older.Time;
			const double AlphaD = (Den > KINDA_SMALL_NUMBER)
				? ((ClampedShotTime - Older.Time) / Den)
				: 0.0;
			const float Alpha = static_cast<float>(FMath::Clamp(AlphaD, 0.0, 1.0));

			OutFrame.Time = ClampedShotTime;
			OutFrame.Bodies.Reserve(Older.Bodies.Num());

			for (const FRewindPhysBodyState& OlderBody : Older.Bodies)
			{
				const FRewindPhysBodyState* YoungerBody = Younger.Bodies.FindByPredicate(
					[&](const FRewindPhysBodyState& Candidate)
					{
						return Candidate.BodyName == OlderBody.BodyName;
					});

				if (!YoungerBody)
				{
					continue;
				}

				const FVector Loc = FMath::Lerp(
					OlderBody.WorldTransform.GetLocation(),
					YoungerBody->WorldTransform.GetLocation(),
					Alpha);

				const FQuat Rot = FQuat::Slerp(
					OlderBody.WorldTransform.GetRotation(),
					YoungerBody->WorldTransform.GetRotation(),
					Alpha).GetNormalized();

				const FVector Scale = FMath::Lerp(
					OlderBody.WorldTransform.GetScale3D(),
					YoungerBody->WorldTransform.GetScale3D(),
					Alpha);

				FRewindPhysBodyState InterpBody;
				InterpBody.BodyName = OlderBody.BodyName;
				InterpBody.WorldTransform = FTransform(Rot, Loc, Scale);

				OutFrame.Bodies.Add(InterpBody);
			}

			return OutFrame.Bodies.Num() > 0;
		}
	}
	return false;
}

void ULagCompensationComponent::EnableRewindCollision(FCollisionBackup& Backup) const
{
	UCapsuleComponent* Capsule = OwnerCharacter->GetCapsuleComponent();
	USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh();

	if (Capsule)
	{
		Backup.bHasCapsule = true;
		Backup.CapsuleEnabled = Capsule->GetCollisionEnabled();

		// Disable capsule so the line trace can hit the rewound bodies/mesh
		Capsule->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	if (Mesh)
	{
		Backup.bHasMesh = true;
		Backup.MeshEnabled = Mesh->GetCollisionEnabled();
		Backup.MeshObjectType = Mesh->GetCollisionObjectType();
		Backup.MeshResponses = Mesh->GetCollisionResponseToChannels();

		Mesh->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		Mesh->SetCollisionResponseToAllChannels(ECR_Ignore);
		Mesh->SetCollisionResponseToChannel(WeaponTraceChannel, ECR_Block);
	}
}

void ULagCompensationComponent::RestoreCollisionFromBackup(const FCollisionBackup& Backup) const
{
	UCapsuleComponent* Capsule = OwnerCharacter->GetCapsuleComponent();
	USkeletalMeshComponent* Mesh = OwnerCharacter->GetMesh();

	if (Capsule && Backup.bHasCapsule)
	{
		Capsule->SetCollisionEnabled(Backup.CapsuleEnabled);
	}

	if (Mesh && Backup.bHasMesh)
	{
		Mesh->SetCollisionObjectType(Backup.MeshObjectType);
		Mesh->SetCollisionResponseToChannels(Backup.MeshResponses);
		Mesh->SetCollisionEnabled(Backup.MeshEnabled);
	}
}

bool ULagCompensationComponent::ConfirmHitRewind(
	ABaseCharacter* Shooter,
	const FVector& TraceStart,
	const FVector& TraceEnd,
	double ShotTime,
	FHitResult& OutHit)
{
	OutHit = FHitResult{};

	if (!OwnerCharacter->HasAuthority())
	{
		return false;
	}
	if (FrameHistory.Num() == 0)
	{
		return false;
	}

	// reject if shot is too old for available history
	const double Newest = FrameHistory[0].Time;
	const double Oldest = FrameHistory.Last().Time;
	if (ShotTime < Oldest - 0.005 || ShotTime > Newest + 0.05)
	{
		return false;
	}

	// Cache current body transforms
	FRewindFrame CachedCurrent;
	CacheCurrentBodies(CachedCurrent);

	// Build interpolated rewind frame
	FRewindFrame RewindFrame;
	if (!GetInterpolatedFrameAtTime(ShotTime, RewindFrame))
	{
		return false;
	}

	// Rewind physics bodies
	MoveBodiesToFrame(RewindFrame);

	// Temporarily switch collision for rewind trace
	/*FCollisionBackup CollisionBackup;
	EnableRewindCollision(CollisionBackup);*/

	FCollisionQueryParams Params(SCENE_QUERY_STAT(RewindPhysTrace), false);
	if (Shooter)
	{
		Params.AddIgnoredActor(Shooter);
	}

	Params.ClearIgnoredSourceObjects();
	if (Shooter)
	{
		Params.AddIgnoredActor(Shooter);
	}

	const bool bHit = GetWorld()->LineTraceSingleByChannel(
		OutHit,
		TraceStart,
		TraceEnd,
		WeaponTraceChannel,
		Params
	);

	// Keep only hits on this owner character
	const bool bValidHit = bHit && (OutHit.GetActor() == OwnerCharacter);

	// Restore current state immediately
	/*RestoreCollisionFromBackup(CollisionBackup);*/
	RestoreBodies(CachedCurrent);

	return bValidHit;
}