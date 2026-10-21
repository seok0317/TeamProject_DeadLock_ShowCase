// 개인 구현 및 수정 범위 전시용. 독립 빌드 대상이 아닙니다.
// 분류: C — 팀원과 기능 공동 관여
// 원본: Source/TeamProject_DL/MovementComponent/Ability/GroundRollAbility.cpp
// 기준 커밋: db9a1720265f38f776d8c1ff003933a356ebeef3
// 기여 범위: 구르기 이동·카메라·점프 허용 구간은 본인. horimi_의 daa94d153 DashBlocked 발동 차단은 기능 변경이므로 공동으로 분류.
// 원본 인코딩: utf-8-sig. UTF-8로 표시하고 BOM·줄바꿈을 정규화했습니다. 원문 주석/로그를 보존합니다.
// 본인 작성·수정행만 발췌했습니다. 함수/클래스 전체가 아니며 조각 사이의 선언·제어 흐름을 재구성하지 않았습니다.

// [원문 1-2] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
#include "../../MovementComponent/Ability/GroundRollAbility.h" 
#include "GameFramework/Character.h"

// [원문 4-15] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h" 
#include "../../GameCore/Character/BaseCharacter.h"
#include "../GameCore/Character/PlayerCharacter.h" 
#include "GameFramework/SpringArmComponent.h" 
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_ApplyRootMotionConstantForce.h" 
#include "GameplayTagContainer.h" 

UGroundRollAbility::UGroundRollAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

// [원문 17-17] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
	// 대상 ASC에 이 태그가 유지되는 동안 지상 대시(구르기)를 새로 발동할 수 없음

// [원문 20-200] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
}

void UGroundRollAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	bIsEnding = false;
	bCameraRestored = false; 

	ABaseCharacter* AvatarCharacter = Cast<ABaseCharacter>(ActorInfo->AvatarActor.Get());
	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(AvatarCharacter);
	UBaseMovementComponent* CMC = AvatarCharacter ? Cast<UBaseMovementComponent>(AvatarCharacter->GetCharacterMovement()) : nullptr;
	UAnimMontage* RollMontage = AvatarCharacter ? AvatarCharacter->GroundRollMontage : nullptr;

	if (AvatarCharacter && CMC && RollMontage)
	{
		FVector RollDir = FVector::ZeroVector;

		if (ActorInfo->IsLocallyControlled())
		{
			RollDir = CMC->GetLastInputVector();
			if (RollDir.IsNearlyZero())
			{
				RollDir = AvatarCharacter->GetActorForwardVector();
			}
		}
		else
		{
			RollDir = AvatarCharacter->GetReplicatedMovementInput();
		}

		RollDir.Normalize();
		CachedRollDir = RollDir;

		CMC->bOrientRotationToMovement = false;
		CMC->bUseControllerDesiredRotation = false;
		AvatarCharacter->SetActorRotation(FRotator(0.0f, CachedRollDir.Rotation().Yaw, 0.0f), ETeleportType::TeleportPhysics);


		if (PlayerCharacter && PlayerCharacter->CameraBoom)
		{
			bCachedCameraLagEnabled = PlayerCharacter->CameraBoom->bEnableCameraLag;
			CachedCameraLagSpeed = PlayerCharacter->CameraBoom->CameraLagSpeed;

			FVector CameraForward = FRotationMatrix(AvatarCharacter->GetControlRotation()).GetUnitAxis(EAxis::X);
			CameraForward.Z = 0.0f;
			float ForwardDot = FVector::DotProduct(CameraForward.GetSafeNormal(), CachedRollDir);

			if (ForwardDot < -0.2f)
			{
				
				RestoreCameraLag();
			}
			else
			{
				
				PlayerCharacter->CameraBoom->bEnableCameraLag = true;
				PlayerCharacter->CameraBoom->CameraLagSpeed = 2.0f;

				UAbilityTask_WaitDelay* CameraLagTimer = UAbilityTask_WaitDelay::WaitDelay(this, 0.1f);
				CameraLagTimer->OnFinish.AddDynamic(this, &UGroundRollAbility::RestoreCameraLag);
				CameraLagTimer->ReadyForActivation();
			}
		}

		
		UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			NAME_None,
			RollMontage,
			1.0f,
			NAME_None
		);
		MontageTask->OnCompleted.AddDynamic(this, &UGroundRollAbility::OnRollFinished);
		MontageTask->OnInterrupted.AddDynamic(this, &UGroundRollAbility::OnRollFinished);
		MontageTask->OnCancelled.AddDynamic(this, &UGroundRollAbility::OnRollFinished);
		MontageTask->ReadyForActivation();

	
		UAbilityTask_ApplyRootMotionConstantForce* RootMotionTask =
			UAbilityTask_ApplyRootMotionConstantForce::ApplyRootMotionConstantForce(
				this,
				TEXT("GroundRollRootMotionForce"),
				CachedRollDir,
				RollSpeed,
				RollDuration,
				true,
				nullptr,
				ERootMotionFinishVelocityMode::SetVelocity,
				FVector(CachedRollDir.X * CMC->MaxWalkSpeed, CachedRollDir.Y * CMC->MaxWalkSpeed, 0.0f),
				0.0f,
				false
			);
		RootMotionTask->ReadyForActivation();

		
		GetWorld()->GetTimerManager().SetTimer(RollJumpWindowTimerHandle, this, &UGroundRollAbility::OpenRollJumpWindow, JumpStartTiming, false);
		GetWorld()->GetTimerManager().SetTimer(RollJumpWindowEndTimerHandle, this, &UGroundRollAbility::CloseRollJumpWindow, JumpEndTiming, false);

	
		UAbilityTask_WaitDelay* RollTimerTask = UAbilityTask_WaitDelay::WaitDelay(this, RollDuration);
		RollTimerTask->OnFinish.AddDynamic(this, &UGroundRollAbility::OnRollDurationFinished);
		RollTimerTask->ReadyForActivation();
	}
	else
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
	}
}


void UGroundRollAbility::RestoreCameraLag()
{
	if (bCameraRestored || !CurrentActorInfo) return;
	bCameraRestored = true;

	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(CurrentActorInfo->AvatarActor.Get());
	if (PlayerCharacter && PlayerCharacter->CameraBoom)
	{
		PlayerCharacter->CameraBoom->bEnableCameraLag = bCachedCameraLagEnabled;
		PlayerCharacter->CameraBoom->CameraLagSpeed = CachedCameraLagSpeed;
	}
}

void UGroundRollAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (bIsEnding) return;
	bIsEnding = true;

	GetWorld()->GetTimerManager().ClearTimer(RollJumpWindowTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(RollJumpWindowEndTimerHandle);

	if (UAbilitySystemComponent* ASC = CurrentActorInfo->AbilitySystemComponent.Get())
	{
		ASC->RemoveLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("State.Movement.CanJump")));
	}

	ABaseCharacter* AvatarCharacter = Cast<ABaseCharacter>(ActorInfo->AvatarActor.Get());
	UBaseMovementComponent* CMC = AvatarCharacter ? Cast<UBaseMovementComponent>(AvatarCharacter->GetCharacterMovement()) : nullptr;

	if (AvatarCharacter && AvatarCharacter->GroundRollMontage)
	{
		AvatarCharacter->StopAnimMontage(AvatarCharacter->GroundRollMontage);
	}

	RestoreCameraLag();

	if (CMC)
	{
		CMC->bOrientRotationToMovement = false;
		CMC->bUseControllerDesiredRotation = true;
		CMC->RotationRate = FRotator(0.0f, 600.0f, 0.0f);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGroundRollAbility::OnRollFinished()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGroundRollAbility::OnRollDurationFinished()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGroundRollAbility::OpenRollJumpWindow()
{
	if (UAbilitySystemComponent* ASC = CurrentActorInfo->AbilitySystemComponent.Get())
	{
		ASC->AddLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("State.Movement.CanJump")));
	}
}

void UGroundRollAbility::CloseRollJumpWindow()
{
	if (UAbilitySystemComponent* ASC = CurrentActorInfo->AbilitySystemComponent.Get())
	{
		ASC->RemoveLooseGameplayTag(FGameplayTag::RequestGameplayTag(FName("State.Movement.CanJump")));
	}

