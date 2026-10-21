// 개인 구현 및 수정 범위 전시용. 독립 빌드 대상이 아닙니다.
// 분류: A — 본인 단독 구현
// 원본: Source/TeamProject_DL/Lash/GA_Lash_DeathSlam.cpp
// 기준 커밋: db9a1720265f38f776d8c1ff003933a356ebeef3
// 기여 범위: 본인이 신규 작성하고 후속 수정한 파일. 일반 커밋·병합 combined diff·현재 blame에서 팀원의 기능 변경을 확인하지 못함.
// 원본 인코딩: utf-8-sig. UTF-8로 표시하고 BOM·줄바꿈을 정규화했습니다. 원문 주석/로그를 보존합니다.
// 원문 전체입니다. 팀원 지원 수정이 있는 B 파일은 위 기여 범위와 SourceManifest에 명시합니다.

#include "GA_Lash_DeathSlam.h"
#include "LashLockonWidget.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/CapsuleComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "AbilityTask_TargetingScope.h"
#include "Abilities/Tasks/AbilityTask_WaitInputPress.h" 
#include "Abilities/Tasks/AbilityTask_ApplyRootMotionConstantForce.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "TeamUtils.h"
#include "Common/SkillDamageLibrary.h"

UGA_Lash_DeathSlam::UGA_Lash_DeathSlam()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	TargetingMode = ESkillTargetingMode::Scope;
	ConfirmMode = ESkillConfirmMode::Immediate;

	bEndAbilityWhenCastMontageEnds = false;
	bEndAbilityWhenCastMontageCancelled = false;

	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("State.Combat.InplaceSkill")));
	BlockAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.BasicAttack")));
	BlockAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Reload")));
}

void UGA_Lash_DeathSlam::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	ACharacter* AvatarChar = Cast<ACharacter>(ActorInfo->AvatarActor.Get());
	if (AvatarChar)
	{
		UCharacterMovementComponent* MoveComp = AvatarChar->GetCharacterMovement();
		UAnimMontage* ScanMontageToPlay = (MoveComp && MoveComp->IsFalling()) ? AirborneScanMontage : GroundScanMontage;
		if (ScanMontageToPlay)
		{
			AvatarChar->PlayAnimMontage(ScanMontageToPlay);
		}
	}

	if (ActorInfo->IsLocallyControlled())
	{
		GetWorld()->GetTimerManager().SetTimer(LockonTimerHandle, this, &UGA_Lash_DeathSlam::UpdateLockonTargets, 0.05f, true);
		GetWorld()->GetTimerManager().SetTimer(ScopeTimeoutTimerHandle, this, &UGA_Lash_DeathSlam::OnScopeTimeout, 5.0f, false);

		UAbilityTask_WaitInputPress* InputTask = UAbilityTask_WaitInputPress::WaitInputPress(this);
		if (InputTask)
		{
			InputTask->OnPress.AddDynamic(this, &UGA_Lash_DeathSlam::OnConfirmPressed);
			InputTask->ReadyForActivation();
		}
	}
}

void UGA_Lash_DeathSlam::OnCastStarted_Implementation()
{
	Super::OnCastStarted_Implementation();
	bHasSlammed = false;
	bHasLaunchedDown = false;

	OnPhase1DataReceived(CachedTargetData, FGameplayTag());
}

void UGA_Lash_DeathSlam::OnSkillEvent_Implementation(FGameplayTag EventTag, FGameplayEventData Payload)
{
	Super::OnSkillEvent_Implementation(EventTag, Payload);
	LaunchEnemiesToGround();
}

void UGA_Lash_DeathSlam::OnScopeTimeout()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}


void UGA_Lash_DeathSlam::UpdateLockonTargets()
{
	if (!CurrentActorInfo->IsLocallyControlled()) return;

	TArray<TObjectPtr<ACharacter>> ScopeTargets;
	for (UGameplayTask* Task : ActiveTasks)
	{
		if (UAbilityTask_TargetingScope* FoundScopeTask = Cast<UAbilityTask_TargetingScope>(Task))
		{
			ScopeTargets = FoundScopeTask->GetCollectedTargets();
			break;
		}
	}

	for (int32 i = TrackedTargets.Num() - 1; i >= 0; --i)
	{
		FLashLockonTarget& TargetInfo = TrackedTargets[i];

		if (ScopeTargets.Contains(TargetInfo.TargetCharacter))
		{
			if (!TargetInfo.bIsLockedOn)
			{
				TargetInfo.GazeDuration += 0.05f;
				if (TargetInfo.GazeDuration >= RequiredGazeTime)
				{
					TargetInfo.bIsLockedOn = true;
				}

				if (ULashLockonWidget* UI = Cast<ULashLockonWidget>(TargetInfo.LockonWidgetComp->GetWidget()))
				{
					UI->UpdateProgress(FMath::Clamp(TargetInfo.GazeDuration / RequiredGazeTime, 0.f, 1.f), TargetInfo.bIsLockedOn);
				}
			}
			ScopeTargets.Remove(TargetInfo.TargetCharacter);
		}
		else
		{
			if (TargetInfo.LockonWidgetComp) TargetInfo.LockonWidgetComp->DestroyComponent();
			TrackedTargets.RemoveAt(i);
		}
	}

	for (ACharacter* NewTarget : ScopeTargets)
	{
		if (!NewTarget || !NewTarget->IsPlayerControlled())
		{
			continue;
		}

		FLashLockonTarget NewTargetInfo;
		NewTargetInfo.TargetCharacter = NewTarget;
		NewTargetInfo.GazeDuration = 0.0f;

		if (LockonWidgetClass)
		{
			UWidgetComponent* WidgetComp = NewObject<UWidgetComponent>(NewTarget);
			WidgetComp->SetWidgetClass(LockonWidgetClass);
			WidgetComp->SetWidgetSpace(EWidgetSpace::Screen);
			WidgetComp->SetDrawSize(FVector2D(100.f, 100.f));
			WidgetComp->SetupAttachment(NewTarget->GetMesh(), FName("head"));
			WidgetComp->SetRelativeLocation(FVector(0, 0, 50.f));
			WidgetComp->RegisterComponent();

			NewTargetInfo.LockonWidgetComp = WidgetComp;
		}
		TrackedTargets.Add(NewTargetInfo);
	}
}

void UGA_Lash_DeathSlam::OnConfirmPressed(float TimeWaited)
{
	if (!CurrentActorInfo->IsLocallyControlled()) return;

	GetWorld()->GetTimerManager().ClearTimer(LockonTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(ScopeTimeoutTimerHandle);

	if (ScopeTask)
	{
		ScopeTask->EndTask();
		ScopeTask = nullptr;
	}

	FGameplayAbilityTargetDataHandle TargetDataHandle;
	FGameplayAbilityTargetData_ActorArray* ActorArrayData = new FGameplayAbilityTargetData_ActorArray();

	for (FLashLockonTarget& TargetInfo : TrackedTargets)
	{
		// 게이지가 꽉 찬(bIsLockedOn) 적만 배열에 추가
		if (TargetInfo.bIsLockedOn && TargetInfo.TargetCharacter)
		{
			ActorArrayData->TargetActorArray.AddUnique(TargetInfo.TargetCharacter);
		}
		if (TargetInfo.LockonWidgetComp) TargetInfo.LockonWidgetComp->DestroyComponent();
	}
	TrackedTargets.Empty();

	// 게이지를 꽉 채운 적이 0명이라면
	// 서버로 타겟 전송을 하지 않고 스킬을 즉시 취소 종료(EndAbility)
	if (ActorArrayData->TargetActorArray.Num() == 0)
	{
		delete ActorArrayData;
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}

	if (!CastMontage && AirborneScanMontage)
	{
		CastMontage = AirborneScanMontage;
	}

	TargetDataHandle.Add(ActorArrayData);
	OnScopeConfirmed(TargetDataHandle);
}

void UGA_Lash_DeathSlam::Server_CancelSkill_Implementation()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UGA_Lash_DeathSlam::OnPhase1DataReceived(const FGameplayAbilityTargetDataHandle& Data, FGameplayTag ApplicationTag)
{
	ConfirmedTargets.Empty();
	if (Data.Num() > 0)
	{
		const FGameplayAbilityTargetData_ActorArray* ActorArray = static_cast<const FGameplayAbilityTargetData_ActorArray*>(Data.Get(0));
		if (ActorArray)
		{
			for (TWeakObjectPtr<AActor> WeakActor : ActorArray->TargetActorArray)
			{
				if (ACharacter* Char = Cast<ACharacter>(WeakActor.Get()))
				{
					if (CurrentActorInfo->IsLocallyControlled())
					{
						if (!Char->IsPlayerControlled())
						{
							continue;
						}
						bool bIsActuallyLockedOn = false;
						for (const FLashLockonTarget& Tracked : TrackedTargets)
						{
							if (Tracked.TargetCharacter == Char && Tracked.bIsLockedOn)
							{
								bIsActuallyLockedOn = true;
								break;
							}
						}
						if (!bIsActuallyLockedOn) continue;
					}

					FLashLockonTarget NewTarget;
					NewTarget.TargetCharacter = Char;
					ConfirmedTargets.Add(NewTarget);
				}
			}
		}
	}

	ACharacter* AvatarChar = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!AvatarChar || ConfirmedTargets.Num() == 0)
	{
		// 클라이언트가 서버에게 유효한 타겟이 0명이니 서버 스킬도 꺼라라고 즉시 통신
		if (CurrentActorInfo->IsLocallyControlled())
		{
			Server_CancelSkill();
		}

		// 내 클라이언트 화면에서도 즉시 스킬을 끔
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		return;
	}
	GetWorld()->GetTimerManager().SetTimer(ServerLaunchTimerHandle, this, &UGA_Lash_DeathSlam::ExecuteServerLaunch, 0.06f, false);
}

void UGA_Lash_DeathSlam::ExecuteServerLaunch()
{
	ACharacter* AvatarChar = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!AvatarChar) return;

	UCharacterMovementComponent* MoveComp = AvatarChar->GetCharacterMovement();
	if (MoveComp)
	{
		MoveComp->StopMovementImmediately();
		MoveComp->GravityScale = 0.0f;
		MoveComp->SetMovementMode(MOVE_Flying);

		UAbilityTask_ApplyRootMotionConstantForce* AscendTask = UAbilityTask_ApplyRootMotionConstantForce::ApplyRootMotionConstantForce(
			this, FName("LashAscend"), FVector::UpVector, AscendHeight * 1.8f, 0.35f, false, nullptr,
			ERootMotionFinishVelocityMode::SetVelocity, FVector::ZeroVector, 0.f, false);

		if (AscendTask)
		{
			AscendTask->OnFinish.AddDynamic(this, &UGA_Lash_DeathSlam::OnAscendFinished);
			AscendTask->ReadyForActivation();
		}
	}

	if (CurrentActorInfo->IsLocallyControlled() && TargetIndicatorFXClass)
	{
		TargetIndicatorFXComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), TargetIndicatorFXClass, AvatarChar->GetActorLocation());
		if (TargetIndicatorFXComp)
		{
			TargetIndicatorFXComp->SetNiagaraVariableFloat(TEXT("Radius"), SlamRadius);
		}
	}

	UAbilitySystemComponent* MyASC = GetAbilitySystemComponentFromActorInfo();
	FVector InitialPullLoc = AvatarChar->GetActorLocation() + FVector(0, 0, AscendHeight) + (AvatarChar->GetActorForwardVector() * HoldForwardDistance);

	for (FLashLockonTarget& TargetInfo : ConfirmedTargets)
	{
		ACharacter* EnemyChar = TargetInfo.TargetCharacter;
		if (!EnemyChar) continue;

		TargetInfo.OriginalPawnCollisionResponse = EnemyChar->GetCapsuleComponent()->GetCollisionResponseToChannel(ECC_Pawn);
		EnemyChar->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);

		if (UCharacterMovementComponent* EnemyMove = EnemyChar->GetCharacterMovement())
		{
			TargetInfo.OriginalMovementMode = EnemyMove->MovementMode;
			EnemyMove->StopMovementImmediately();
			EnemyMove->GravityScale = 0.0f;
			EnemyMove->SetMovementMode(MOVE_Flying);
		}

		if (PullRopeFXClass)
		{
			TargetInfo.PullRopeFX = UNiagaraFunctionLibrary::SpawnSystemAttached(PullRopeFXClass, AvatarChar->GetMesh(), FName("Hand_R"), FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::SnapToTarget, true);
		}

		if (HasAuthority(&CurrentActivationInfo))
		{
			UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(EnemyChar);
			if (TargetASC && SuspendStunEffectClass)
			{
				FGameplayEffectContextHandle Context = MyASC->MakeEffectContext();
				Context.AddInstigator(AvatarChar, AvatarChar);
				TargetInfo.SuspendStunGEHandle = TargetASC->ApplyGameplayEffectToSelf(SuspendStunEffectClass->GetDefaultObject<UGameplayEffect>(), 1.0f, Context);
			}

			FVector DirToHover = (InitialPullLoc - EnemyChar->GetActorLocation()).GetSafeNormal();
			EnemyChar->LaunchCharacter(DirToHover * 1200.0f + FVector(0, 0, 350.0f), true, true);
		}
	}

	GetWorld()->GetTimerManager().SetTimer(HoldEnemiesTimerHandle, this, &UGA_Lash_DeathSlam::UpdateHoldingAndVisualFX, 0.03f, true);
	GetWorld()->GetTimerManager().SetTimer(HoverTimerHandle, this, &UGA_Lash_DeathSlam::CalculateAndSendSlamTarget, 1.2f, false);
}

void UGA_Lash_DeathSlam::OnAscendFinished()
{
	if (ACharacter* AvatarChar = Cast<ACharacter>(GetAvatarActorFromActorInfo()))
	{
		AvatarChar->GetCharacterMovement()->Velocity = FVector::ZeroVector;
	}
}

void UGA_Lash_DeathSlam::UpdateHoldingAndVisualFX()
{
	ACharacter* AvatarChar = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	if (!AvatarChar) return;

	if (HasAuthority(&CurrentActivationInfo))
	{
		FVector HoldCenter = AvatarChar->GetActorLocation() + (AvatarChar->GetActorForwardVector() * HoldForwardDistance) + FVector(0, 0, HoldHeightOffset);
		for (int32 i = 0; i < ConfirmedTargets.Num(); ++i)
		{
			ACharacter* EnemyChar = ConfirmedTargets[i].TargetCharacter;
			if (!EnemyChar) continue;

			FVector SpreadOffset = FVector(0.f, (i - (ConfirmedTargets.Num() - 1) * 0.5f) * 110.f, 0.f);
			FVector TargetHoldPos = HoldCenter + AvatarChar->GetTransform().TransformVectorNoScale(SpreadOffset);

			FVector ToTarget = (TargetHoldPos - EnemyChar->GetActorLocation());
			if (UCharacterMovementComponent* EnemyMove = EnemyChar->GetCharacterMovement())
			{
				EnemyMove->Velocity = ToTarget * 12.0f;
			}
		}
	}

	for (FLashLockonTarget& TargetInfo : ConfirmedTargets)
	{
		if (TargetInfo.PullRopeFX && TargetInfo.TargetCharacter)
		{
			TargetInfo.PullRopeFX->SetNiagaraVariableVec3(
				TEXT("TargetEnd"),
				TargetInfo.TargetCharacter->GetActorLocation() + FVector(0.f, 0.f, 50.f)
			);
		}
	}

	if (CurrentActorInfo->IsLocallyControlled() && TargetIndicatorFXComp)
	{
		APlayerController* PC = Cast<APlayerController>(AvatarChar->GetController());
		if (PC && PC->PlayerCameraManager)
		{
			FVector CameraLoc = PC->PlayerCameraManager->GetCameraLocation();
			FVector CameraDir = PC->PlayerCameraManager->GetCameraRotation().Vector();
			FVector TraceEnd = CameraLoc + (CameraDir * SkillRange);

			FHitResult AimHit;
			FCollisionQueryParams Params;
			Params.AddIgnoredActor(AvatarChar);

			FVector AimPoint = TraceEnd;
			if (GetWorld()->LineTraceSingleByChannel(AimHit, CameraLoc, TraceEnd, ECC_Visibility, Params))
			{
				AimPoint = AimHit.Location;
			}

			FVector DownwardStart = FVector(AimPoint.X, AimPoint.Y, AimPoint.Z + 500.f);
			FVector DownwardEnd = FVector(AimPoint.X, AimPoint.Y, AimPoint.Z - 10000.f);
			FHitResult GroundHit;
			FVector IndicatorLoc = AimPoint;
			if (GetWorld()->LineTraceSingleByChannel(GroundHit, DownwardStart, DownwardEnd, ECC_Visibility, Params))
			{
				IndicatorLoc = GroundHit.Location;
			}
			else
			{
				IndicatorLoc.Z = AvatarChar->GetActorLocation().Z - AvatarChar->GetDefaultHalfHeight();
			}

			IndicatorLoc += FVector(0.f, 0.f, 15.f);
			TargetIndicatorFXComp->SetWorldLocation(IndicatorLoc);
		}
	}
}

// ==========================================
// ☄️ [Phase 3] 클라이언트 좌표 전송
// ==========================================
void UGA_Lash_DeathSlam::CalculateAndSendSlamTarget()
{
	if (!CurrentActorInfo->IsLocallyControlled()) return;

	ACharacter* AvatarChar = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	APlayerController* PC = Cast<APlayerController>(AvatarChar->GetController());
	if (!AvatarChar || !PC || !PC->PlayerCameraManager) return;

	FVector CameraLoc = PC->PlayerCameraManager->GetCameraLocation();
	FVector CameraDir = PC->PlayerCameraManager->GetCameraRotation().Vector();
	FVector TraceEnd = CameraLoc + (CameraDir * SkillRange);

	FHitResult AimHit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(AvatarChar);

	FVector AimPoint = TraceEnd;
	if (GetWorld()->LineTraceSingleByChannel(AimHit, CameraLoc, TraceEnd, ECC_Visibility, Params))
	{
		AimPoint = AimHit.Location;
	}

	FVector DownwardStart = FVector(AimPoint.X, AimPoint.Y, AimPoint.Z + 500.f);
	FVector DownwardEnd = FVector(AimPoint.X, AimPoint.Y, AimPoint.Z - 10000.f);
	FHitResult GroundHit;
	FVector CalculatedTargetLoc = AimPoint;
	if (GetWorld()->LineTraceSingleByChannel(GroundHit, DownwardStart, DownwardEnd, ECC_Visibility, Params))
	{
		CalculatedTargetLoc = GroundHit.Location;
	}
	else
	{
		CalculatedTargetLoc.Z = AvatarChar->GetActorLocation().Z - AvatarChar->GetDefaultHalfHeight();
	}

	Server_SendSlamTargetLoc(CalculatedTargetLoc);
}

void UGA_Lash_DeathSlam::Server_SendSlamTargetLoc_Implementation(FVector TargetLoc)
{
	ProcessSlamDive(TargetLoc);
}

void UGA_Lash_DeathSlam::ProcessSlamDive(FVector TargetLoc)
{
	FinalTargetLocation = TargetLoc;

	GetWorld()->GetTimerManager().ClearTimer(HoldEnemiesTimerHandle);

	if (TargetIndicatorFXComp)
	{
		TargetIndicatorFXComp->SetWorldLocation(FinalTargetLocation);
	}

	if (HasAuthority(&CurrentActivationInfo))
	{
		for (FLashLockonTarget& TargetInfo : ConfirmedTargets)
		{
			if (TargetInfo.TargetCharacter && TargetInfo.TargetCharacter->GetCharacterMovement())
			{
				TargetInfo.TargetCharacter->GetCharacterMovement()->Velocity = FVector::ZeroVector;
			}
		}
	}

	if (CastMontage)
	{
		UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this, NAME_None, CastMontage, 1.0f);
		if (MontageTask) MontageTask->ReadyForActivation();
	}

	GetWorld()->GetTimerManager().SetTimer(SlamDiveTimerHandle, this, &UGA_Lash_DeathSlam::LaunchEnemiesToGround, SlamDiveDelay, false);
}

void UGA_Lash_DeathSlam::LaunchEnemiesToGround()
{
	if (bHasLaunchedDown) return;
	bHasLaunchedDown = true;

	GetWorld()->GetTimerManager().ClearTimer(SlamDiveTimerHandle);

	if (HasAuthority(&CurrentActivationInfo))
	{
		for (int32 i = 0; i < ConfirmedTargets.Num(); ++i)
		{
			ACharacter* EnemyChar = ConfirmedTargets[i].TargetCharacter;
			if (!EnemyChar) continue;

			FVector Offset = FVector(FMath::Cos((float)i) * 90.f, FMath::Sin((float)i) * 90.f, 0.f);
			FVector DiveTarget = FinalTargetLocation + Offset;

			FVector StartLoc = EnemyChar->GetActorLocation();
			FVector DiveDir = (DiveTarget - StartLoc).GetSafeNormal();

			if (UCharacterMovementComponent* EnemyMove = EnemyChar->GetCharacterMovement())
			{
				EnemyMove->Velocity = FVector::ZeroVector;
				EnemyMove->SetMovementMode(MOVE_Falling);
			}
			EnemyChar->LaunchCharacter(DiveDir * 4500.0f, true, true);
		}
	}

	SlamFlightTime = 0.0f;
	GetWorld()->GetTimerManager().SetTimer(GroundSlamTimerHandle, this, &UGA_Lash_DeathSlam::CheckSlamImpact, 0.03f, true);
}

void UGA_Lash_DeathSlam::CheckSlamImpact()
{
	SlamFlightTime += 0.03f;
	bool bAnyHitGround = false;

	for (FLashLockonTarget& TargetInfo : ConfirmedTargets)
	{
		if (TargetInfo.PullRopeFX && TargetInfo.TargetCharacter)
		{
			TargetInfo.PullRopeFX->SetNiagaraVariableVec3(
				TEXT("TargetEnd"),
				TargetInfo.TargetCharacter->GetActorLocation() + FVector(0.f, 0.f, 50.f)
			);
		}

		ACharacter* EnemyChar = TargetInfo.TargetCharacter;
		if (!EnemyChar) continue;

		float DistZ = FMath::Abs(EnemyChar->GetActorLocation().Z - FinalTargetLocation.Z);
		if (EnemyChar->GetCharacterMovement()->IsMovingOnGround() || DistZ <= 130.0f)
		{
			bAnyHitGround = true;
		}
	}

	if (bAnyHitGround || SlamFlightTime >= 0.8f)
	{
		GetWorld()->GetTimerManager().ClearTimer(GroundSlamTimerHandle);
		ExecuteSlamImpact();
	}
}

void UGA_Lash_DeathSlam::ExecuteSlamImpact()
{
	if (bHasSlammed) return;
	bHasSlammed = true;

	ACharacter* AvatarChar = Cast<ACharacter>(GetAvatarActorFromActorInfo());
	UWorld* World = GetWorld();
	if (!AvatarChar || !World) return;

	if (TargetIndicatorFXComp)
	{
		TargetIndicatorFXComp->DestroyComponent();
		TargetIndicatorFXComp = nullptr;
	}

	UAbilitySystemComponent* MyASC = GetAbilitySystemComponentFromActorInfo();

	for (FLashLockonTarget& TargetInfo : ConfirmedTargets)
	{
		ACharacter* SlammedEnemy = TargetInfo.TargetCharacter;
		if (!SlammedEnemy) continue;

		FVector ImpactCenter = SlammedEnemy->GetActorLocation() + FVector(0.f, 0.f, 30.f);

		// 태그와 위치 정보를 담아 큐를 실행
		if (MyASC && SlamImpactCueTag.IsValid())
		{
			FGameplayCueParameters CueParams;
			CueParams.Location = ImpactCenter;                 // VFX가 생성될 위치
			CueParams.Instigator = AvatarChar;                 // 시전 자 
			CueParams.EffectCauser = AvatarChar;
			CueParams.TargetAttachComponent = SlammedEnemy->GetRootComponent();

			MyASC->ExecuteGameplayCue(SlamImpactCueTag, CueParams);
		}

		if (HasAuthority(&CurrentActivationInfo))
		{
			TArray<AActor*> DamagedActors;

			TArray<FHitResult> HitResults;
			FCollisionShape SphereShape = FCollisionShape::MakeSphere(SlamRadius);
			FCollisionQueryParams Params(SCENE_QUERY_STAT(SlamImpact), false, AvatarChar);

			bool bHit = World->SweepMultiByChannel(HitResults, ImpactCenter, ImpactCenter, FQuat::Identity, ECC_Pawn, SphereShape, Params);
			if (bHit)
			{
				for (const FHitResult& Hit : HitResults)
				{
					AActor* HitActor = Hit.GetActor();
					if (HitActor && SkillTeamUtils::IsHostile(AvatarChar, HitActor) && !DamagedActors.Contains(HitActor))
					{
						DamagedActors.Add(HitActor);
						USkillDamageLibrary::ApplyDamage(AvatarChar, HitActor, SpiritCoefficient, BaseDamage, TAG_Data_Damage_Spirit);
					}
				}
			}
		}
	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_Lash_DeathSlam::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(LockonTimerHandle);
		World->GetTimerManager().ClearTimer(HoverTimerHandle);
		World->GetTimerManager().ClearTimer(HoldEnemiesTimerHandle);
		World->GetTimerManager().ClearTimer(SlamDiveTimerHandle);
		World->GetTimerManager().ClearTimer(GroundSlamTimerHandle);
		World->GetTimerManager().ClearTimer(ScopeTimeoutTimerHandle);
		World->GetTimerManager().ClearTimer(ServerLaunchTimerHandle);
	}

	if (TargetIndicatorFXComp)
	{
		TargetIndicatorFXComp->DestroyComponent();
		TargetIndicatorFXComp = nullptr;
	}

	if (ACharacter* AvatarChar = Cast<ACharacter>(GetActorInfo().AvatarActor.Get()))
	{
		if (UCharacterMovementComponent* MoveComp = AvatarChar->GetCharacterMovement())
		{
			MoveComp->GravityScale = 1.0f;
			if (MoveComp->MovementMode == MOVE_Flying)
			{
				MoveComp->SetMovementMode(MOVE_Falling);
			}
		}
	}

	// TrackedTargets에 들어있던 모든 적들의 위젯을 제거하고 배열을 비움
	for (FLashLockonTarget& TargetInfo : TrackedTargets)
	{
		if (TargetInfo.LockonWidgetComp)
		{
			TargetInfo.LockonWidgetComp->DestroyComponent();
		}
	}
	TrackedTargets.Empty();

	for (FLashLockonTarget& TargetInfo : ConfirmedTargets)
	{
		if (TargetInfo.LockonWidgetComp) TargetInfo.LockonWidgetComp->DestroyComponent();
		if (TargetInfo.PullRopeFX) TargetInfo.PullRopeFX->DestroyComponent();

		if (TargetInfo.TargetCharacter)
		{
			TargetInfo.TargetCharacter->GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Pawn, TargetInfo.OriginalPawnCollisionResponse);

			if (UCharacterMovementComponent* EnemyMove = TargetInfo.TargetCharacter->GetCharacterMovement())
			{
				EnemyMove->StopMovementImmediately();
				EnemyMove->GravityScale = 1.0f;
				EnemyMove->SetMovementMode(TargetInfo.OriginalMovementMode);
			}

			if (TargetInfo.SuspendStunGEHandle.IsValid())
			{
				if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetInfo.TargetCharacter))
				{
					TargetASC->RemoveActiveGameplayEffect(TargetInfo.SuspendStunGEHandle);
				}
			}
		}
	}
	ConfirmedTargets.Empty();

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}