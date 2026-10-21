// 개인 구현 및 수정 범위 전시용. 독립 빌드 대상이 아닙니다.
// 분류: C — 팀원과 기능 공동 관여
// 원본: Source/TeamProject_DL/MovementComponent/Ability/AirDashAbility.cpp
// 기준 커밋: db9a1720265f38f776d8c1ff003933a356ebeef3
// 기여 범위: 대시 방향·관성·몽타주·종료 처리는 본인. horimi_가 daa94d153에서 DashBlocked 발동 차단을 추가했으므로 기능 공동 관여. include 정리도 별도 존재.
// 원본 인코딩: utf-8-sig. UTF-8로 표시하고 BOM·줄바꿈을 정규화했습니다. 원문 주석/로그를 보존합니다.
// 본인 작성·수정행만 발췌했습니다. 함수/클래스 전체가 아니며 조각 사이의 선언·제어 흐름을 재구성하지 않았습니다.

// [원문 2-2] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
#include "GameFramework/Character.h"

// [원문 4-4] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

// [원문 6-15] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Logging/LogMacros.h"
#include "Abilities/Tasks/AbilityTask_ApplyRootMotionConstantForce.h" 
#include "GameplayTagContainer.h" 

UAirDashAbility::UAirDashAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

// [원문 20-183] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
	CancelAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Movement.DoubleJump")));
	CancelAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Movement.WallJump")));
	CancelAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Movement.WallCling")));
}

void UAirDashAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	bIsEnding = false;

	ABaseCharacter* AvatarCharacter = Cast<ABaseCharacter>(ActorInfo->AvatarActor.Get());
	UBaseMovementComponent* CMC = AvatarCharacter ? Cast<UBaseMovementComponent>(AvatarCharacter->GetCharacterMovement()) : nullptr;

	if (AvatarCharacter && CMC && AvatarCharacter->AirDashMontage)
	{
		UE_LOG(LogTemp, Warning, TEXT(" AirDashAbility -> ActivateAbility"));

		FVector DashDir = FVector::ZeroVector;

		if (ActorInfo->IsLocallyControlled())
		{
			DashDir = CMC->GetLastInputVector();
			if (DashDir.IsNearlyZero())
			{
				DashDir = AvatarCharacter->GetActorForwardVector();
			}
		}
		else
		{
			DashDir = AvatarCharacter->GetReplicatedMovementInput();
		}

		DashDir.Normalize();
		CachedDashDir = DashDir;

		FVector ForwardVec = AvatarCharacter->GetActorForwardVector();
		FVector RightVec = AvatarCharacter->GetActorRightVector();

		float ForwardDot = FVector::DotProduct(CachedDashDir, ForwardVec);
		float RightDot = FVector::DotProduct(CachedDashDir, RightVec);

		FName SectionName = FName("Forward");
		if (ForwardDot > 0.5f) { SectionName = FName("Forward"); }
		else if (ForwardDot < -0.5f) { SectionName = FName("Backward"); }
		else if (RightDot > 0.5f) { SectionName = FName("Right"); }
		else if (RightDot < -0.5f) { SectionName = FName("Left"); }

		CMC->PrepareAirDash(CachedDashDir);

		UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this, NAME_None, AvatarCharacter->AirDashMontage, 1.0f, SectionName);

		MontageTask->OnCompleted.AddDynamic(this, &UAirDashAbility::OnMontageCompleted);
		MontageTask->OnInterrupted.AddDynamic(this, &UAirDashAbility::OnMontageCompleted);
		MontageTask->OnCancelled.AddDynamic(this, &UAirDashAbility::OnMontageCompleted);
		MontageTask->ReadyForActivation();

		UAbilityTask_WaitDelay* PrepTask = UAbilityTask_WaitDelay::WaitDelay(this, DashPrepDuration);
		PrepTask->OnFinish.AddDynamic(this, &UAirDashAbility::OnDashPrepFinished);
		PrepTask->ReadyForActivation();
	}
	else
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
	}
}

void UAirDashAbility::OnDashPrepFinished()
{
	ABaseCharacter* AvatarCharacter = Cast<ABaseCharacter>(CurrentActorInfo->AvatarActor.Get());
	UBaseMovementComponent* CMC = AvatarCharacter ? Cast<UBaseMovementComponent>(AvatarCharacter->GetCharacterMovement()) : nullptr;

	if (AvatarCharacter && CMC)
	{
		FVector PreDashVelocity = CMC->GetCachedPreDashVelocity();

		FVector CombinedVelocity = PreDashVelocity + (CachedDashDir * DashSpeed);
		CombinedVelocity.Z = 0.0f;

		float FinalDashSpeed = CombinedVelocity.Size();
		if (FinalDashSpeed > DashSpeed)
		{
			// 방향(대각선 궤적)은 100% 보존한 채, 속도의 크기만 DashSpeed로 제한
			CombinedVelocity = CombinedVelocity.GetSafeNormal() * DashSpeed;
			FinalDashSpeed = DashSpeed;
		}

		FVector FinalDashDir = CombinedVelocity.GetSafeNormal();

		UAbilityTask_ApplyRootMotionConstantForce* RootMotionTask =
			UAbilityTask_ApplyRootMotionConstantForce::ApplyRootMotionConstantForce(
				this,
				TEXT("AirDashRootMotionForce"),
				FinalDashDir,
				FinalDashSpeed,
				DashDuration,         
				false,                  
				nullptr,                  


				ERootMotionFinishVelocityMode::SetVelocity,
				FVector(FinalDashDir.X * CMC->MaxWalkSpeed, FinalDashDir.Y * CMC->MaxWalkSpeed, 0.0f),
				0.0f,                   
				false                  
			);


		RootMotionTask->OnFinish.AddDynamic(this, &UAirDashAbility::OnMontageCompleted);
		RootMotionTask->ReadyForActivation();
	}
	else
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
	}
}


void UAirDashAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (bIsEnding) return;

	bIsEnding = true;

	ABaseCharacter* AvatarCharacter = Cast<ABaseCharacter>(ActorInfo->AvatarActor.Get());
	UBaseMovementComponent* CMC = AvatarCharacter ? Cast<UBaseMovementComponent>(AvatarCharacter->GetCharacterMovement()) : nullptr;
	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();

	if (CMC)
	{
		
		CMC->ResetGravity();
	}

	if (AvatarCharacter && AvatarCharacter->AirDashMontage)
	{
		AvatarCharacter->StopAnimMontage(AvatarCharacter->AirDashMontage);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	if (ASC && AvatarCharacter)
	{
		if (!bWasCancelled)
		{
			FGameplayTag HasAirDashedTag = FGameplayTag::RequestGameplayTag(FName("State.Movement.HasAirDashed"), false);
			if (HasAirDashedTag.IsValid())
			{
				ASC->SetLooseGameplayTagCount(FGameplayTag::RequestGameplayTag(FName("State.Movement.HasAirDashed")), 1);
			}
		}

		FGameplayTag JumpQueuedTag = FGameplayTag::RequestGameplayTag(FName("Event.Input.JumpQueued"), false);
		if (JumpQueuedTag.IsValid() && ASC->HasMatchingGameplayTag(JumpQueuedTag))
		{
			ASC->SetLooseGameplayTagCount(JumpQueuedTag, 0);
			AvatarCharacter->Jump();
		}
	}
}

void UAirDashAbility::OnMontageCompleted()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);

