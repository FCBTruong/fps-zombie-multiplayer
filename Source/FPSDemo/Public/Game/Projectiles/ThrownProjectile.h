#pragma once

#include "CoreMinimal.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "ThrownProjectile.generated.h"

class UThrowableConfig;
UCLASS()
class FPSDEMO_API AThrownProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	AThrownProjectile();
	void InitFromData(const UThrowableConfig* InData);
	void LaunchProjectile(FVector LaunchVelocity, AActor* InstigatorActor);

	UFUNCTION(NetMulticast, Unreliable)
	virtual void MulticastExplode(const FVector& Location);

	UFUNCTION(NetMulticast, Unreliable) void MulticastInitData(EItemId ItemId);
	const UThrowableConfig* GetWeaponData() const { return Data; }
protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void ExplodeNow();
	virtual void OnExplode();

	UFUNCTION()
	void OnProjectileHit(
		UPrimitiveComponent* HitComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, FVector NormalImpulse,
		const FHitResult& Hit);

	UPROPERTY(Replicated)
	bool bIsExploded = false;

	UPROPERTY(Replicated)
	bool bDidHit = false;

	UPROPERTY() 
	USphereComponent* Collision;

	UPROPERTY() 
	UProjectileMovementComponent* Projectile;

	UPROPERTY() 
	UStaticMeshComponent* WeaponMesh;

	UPROPERTY()
	TObjectPtr<const UThrowableConfig> Data = nullptr;

	FTimerHandle TimerHandle_Explode;
};