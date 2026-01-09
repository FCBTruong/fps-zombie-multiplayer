// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/ZombieMode.h"
#include "Game/ShooterGameState.h"
#include "Characters/BaseCharacter.h"
#include "Components/RoleComponent.h"
#include "Game/ActorManager.h"
#include "GameFramework/PlayerStart.h"

void AZombieMode::StartPlay()
{
	UE_LOG(LogTemp, Warning, TEXT("SpikeMode Game Started!"));
	Super::StartPlay();

	SpawnBot("B");
	SpawnBot("B");
	SpawnBot("B");
	SpawnBot("B");
}

void AZombieMode::StartRound()
{
	UE_LOG(LogTemp, Warning, TEXT("AZombieMode: Starting Round..."));
	Super::StartRound();

	AShooterGameState* GS = GetGameState<AShooterGameState>();
	if (GS) {
		GS->SetMatchState(EMyMatchState::BUY_PHASE);
	}

	int BuyTime = 3; // seconds
	int TimeBuyEnd = GetWorld()->GetTimeSeconds() + BuyTime;
	GS->SetRoundEndTime(TimeBuyEnd);
	ResetPlayers();

	BotManager->OnStartRoundZombieMode();

	GetWorldTimerManager().SetTimer(
		RoleAssignTimerHandle,
		this,
		&AZombieMode::AssignZombieRoles,
		BuyTime,
		false
	);
}

void AZombieMode::AssignZombieRoles()
{
	AShooterGameState* GS = GetGameState<AShooterGameState>();
	if (!GS) return;
	GS->SetMatchState(EMyMatchState::ROUND_IN_PROGRESS);
	int RoundProgressTime = 180; // seconds - 3 minutes
	int RoundProgressTimeEnd = GetWorld()->GetTimeSeconds() + RoundProgressTime;
	GS->SetRoundEndTime(RoundProgressTimeEnd);

	auto PlayerStates = GS->PlayerArray;

	// Build eligible list (valid PS + valid pawn + valid role comp)
	TArray<AMyPlayerState*> Eligible;
	Eligible.Reserve(PlayerStates.Num());

	for (APlayerState* PS : PlayerStates)
	{
		AMyPlayerState* MyPS = Cast<AMyPlayerState>(PS);
		if (!MyPS) continue;

		ABaseCharacter* MyChar = Cast<ABaseCharacter>(MyPS->GetPawn());
		if (!MyChar) continue;

		URoleComponent* RoleComp = MyChar->GetRoleComponent();
		if (!RoleComp) continue;

		Eligible.Add(MyPS);
	}

	if (Eligible.Num() == 0)
	{
		return;
	}

	const int32 ZombieIdx = FMath::RandRange(0, Eligible.Num() - 1);
	AMyPlayerState* ZombiePS = Eligible[ZombieIdx];
	if (!ZombiePS) return;

	AController* Ctrl = Cast<AController>(ZombiePS->GetOwner());
	if (!Ctrl) return;
	BecomeZombie(Ctrl);
}

void AZombieMode::BecomeZombie(AController* Controller) {
	UE_LOG(LogTemp, Warning, TEXT("AZombieMode::BecomeZombie called"));
	if (ABaseCharacter* Character = Cast<ABaseCharacter>(Controller->GetPawn()))
	{		
		URoleComponent* RoleComp = Character->GetRoleComponent();
		
		if (RoleComp)
		{
			RoleComp->SetRoleAuthoritative(ECharacterRole::Zombie);
		}
	}
	if (ABotAIController* BotCtrl = Cast<ABotAIController>(Controller))
	{
		BotManager->NotifyCharacterRole(BotCtrl, ECharacterRole::Zombie);
	}
}

void AZombieMode::EndRound(FName WinningTeam)
{
	Super::EndRound(WinningTeam);
	UE_LOG(LogTemp, Warning, TEXT("Round Ended! Team %s wins!"), *WinningTeam.ToString());
	AShooterGameState* GS = GetGameState<AShooterGameState>();
	if (GS)
	{
		GS->SetMatchState(EMyMatchState::ROUND_ENDED);
		GS->Multicast_RoundResult(WinningTeam);
	}
}

void AZombieMode::OnCharacterKilled(class AController* Killer, class ABaseCharacter* VictimPawn, const UItemConfig* DamageCauser, bool bWasHeadShot)
{
	Super::OnCharacterKilled(Killer, VictimPawn, DamageCauser, bWasHeadShot);
	UE_LOG(LogTemp, Warning, TEXT("AZombieMode::OnCharacterKilled called"));

	AController* VictimCtr = VictimPawn->GetController();

	ECharacterRole CurrentRole = VictimPawn->GetCharacterRole();
	if (CurrentRole == ECharacterRole::Human)
	{
		// infected
		BecomeZombie(VictimCtr);
		return;
	}
	
	VictimPawn->ApplyRealDeath(false);

	// spawn victim as zombie
	if (CurrentRole == ECharacterRole::Hero)
	{
		// end game
		EndRound("B"); // zombie team wins
		return; // already a zombie
	}
	UE_LOG(LogTemp, Warning, TEXT("Respawning player as zombie..."));

	TWeakObjectPtr<ABaseCharacter> VictimPawnWeak = VictimPawn;
	TWeakObjectPtr<AController>   VictimCtrlWeak = VictimCtr;
	UE_LOG(LogTemp, Warning, TEXT("AZombieMode: Calling BecomeZombie immediately."));


	// if is already a zombie, use logic revive instead
	float RespawnDelay = 2.f; // seconds 
	FTimerDelegate Del;
	Del.BindLambda([this, VictimPawnWeak]()
		{
			UE_LOG(LogTemp, Warning, TEXT("AZombieMode:Respawn timer triggered."));
			if (!VictimPawnWeak.IsValid())
			{
				UE_LOG(LogTemp, Warning, TEXT("AZombieMode: VictimPawn is no longer valid. Cannot Respawn."));
				return;
			}
			ABaseCharacter* VictimPawn = VictimPawnWeak.Get();
			this->ReviveZombie(VictimPawn);
		});

	FTimerHandle Handle;
	GetWorldTimerManager().SetTimer(Handle, Del, RespawnDelay, false);
}

AActor* AZombieMode::ChoosePlayerStart_Implementation(AController* Player)
{
	UE_LOG(LogTemp, Warning, TEXT("DEBUGXX: ChoosePlayerStart_Implementation..."));
	AMyPlayerState* PS = Player->GetPlayerState<AMyPlayerState>();

	if (!PS) {
		UE_LOG(LogTemp, Warning, TEXT("PlayerState is null in ChoosePlayerStart_Implementation"));
		return Super::ChoosePlayerStart_Implementation(Player); // fallback
	}

	AActorManager* AM = AActorManager::Get(GetWorld());

	APlayerStart* PlayerStart = AM->GetRandomZombieStart();
	if (PlayerStart) {
		return PlayerStart;
	}

	return Super::ChoosePlayerStart_Implementation(Player); // fallback
}

APawn* AZombieMode::SpawnDefaultPawnAtTransform_Implementation(
	AController* NewPlayer,
	const FTransform& SpawnTransform
)
{
	FTransform NewTransform = SpawnTransform;

	FRotator Rot = NewTransform.GetRotation().Rotator();
	Rot.Yaw = FMath::RandRange(0.f, 360.f);
	NewTransform.SetRotation(Rot.Quaternion());

	return Super::SpawnDefaultPawnAtTransform_Implementation(NewPlayer, NewTransform);
}

void AZombieMode::ReviveZombie(ABaseCharacter* ZombieCharacter)
{
	if (!ZombieCharacter) return;

	// teleport to new location
	AController* VictimCtrl = ZombieCharacter->GetController();
	if (VictimCtrl)
	{
		AActor* StartSpot = ChoosePlayerStart_Implementation(VictimCtrl);
		if (StartSpot)
		{
			const FVector Loc = StartSpot->GetActorLocation();
			const FRotator Rot = StartSpot->GetActorRotation();

			ZombieCharacter->TeleportTo(Loc, Rot, false, true);
			ZombieCharacter->ForceNetUpdate();
		}
	}

	ZombieCharacter->Revive();
}