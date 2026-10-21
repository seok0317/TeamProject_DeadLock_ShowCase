// 개인 구현 및 수정 범위 전시용. 독립 빌드 대상이 아닙니다.
// 분류: A — 본인 단독 구현
// 원본: Source/TeamProject_DL/MovementComponent/Ability/WallJumpAbility.cpp
// 기준 커밋: db9a1720265f38f776d8c1ff003933a356ebeef3
// 기여 범위: 본인이 신규 작성하고 후속 수정한 파일. 일반 커밋·병합 combined diff·현재 blame에서 팀원의 기능 변경을 확인하지 못함.
// 원본 인코딩: utf-8-sig. UTF-8로 표시하고 BOM·줄바꿈을 정규화했습니다. 원문 주석/로그를 보존합니다.
// 원문 전체입니다. 팀원 지원 수정이 있는 B 파일은 위 기여 범위와 SourceManifest에 명시합니다.

#include "../../MovementComponent/Ability/WallJumpAbility.h" 
#include "GameFramework/Character.h"
#include "../../GameCore/Character/BaseCharacter.h"
#include "../../MovementComponent/BaseMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

UWallJumpAbility::UWallJumpAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Movement.WallJump")));
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
	CancelAbilitiesWithTag.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Movement.WallCling")));

	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("State.Movement.WallJumping")));
}

void UWallJumpAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	ABaseCharacter* AvatarCharacter = Cast<ABaseCharacter>(ActorInfo->AvatarActor.Get());
	UAbilitySystemComponent* ASC = ActorInfo->AbilitySystemComponent.Get();
	UBaseMovementComponent* CMC = AvatarCharacter ? Cast<UBaseMovementComponent>(AvatarCharacter->GetCharacterMovement()) : nullptr;

	if (AvatarCharacter && ASC && CMC)
	{
		// 1. 벽 매달리기 상태 강제 해제
		CMC->ReleaseWallCling();

		// 2. 방향 동기화 및 계산
		FVector InputDir = FVector::ZeroVector;
		if (ActorInfo->IsLocallyControlled()) { InputDir = CMC->GetLastInputVector(); }
		else { InputDir = AvatarCharacter->GetReplicatedMovementInput(); }
		InputDir.Z = 0.0f;
		InputDir.Normalize();

		// 3. 소스 엔진 물리: 벽 반발력 + 방향키 관성력 합산
		FVector WallNormal = CMC->GetWallNormal();
		WallNormal.Z = 0.0f;
		FVector PushVelocity = WallNormal * WallJumpPushForce;
		FVector MomentumVelocity = InputDir * WallJumpMomentumForce;
		FVector FinalVelocity = PushVelocity + MomentumVelocity;

		FGameplayTag HasWallJumpedTag = FGameplayTag::RequestGameplayTag(FName("State.Movement.HasWallJumped"));

		// 이전에 벽점프를 한 적이 없는지 확인
		if (!ASC->HasMatchingGameplayTag(HasWallJumpedTag))
		{
			// 1회차 벽점프
			FinalVelocity.Z = WallJumpUpForce;

			// 2회차 인식 태그 부착
			ASC->AddLooseGameplayTag(HasWallJumpedTag);

		}
		else
		{
			FinalVelocity.Z = 0.0f; // 
		}

		AvatarCharacter->LaunchCharacter(FinalVelocity, true, true);

		GetWorld()->GetTimerManager().SetTimer(AutoMantleTimerHandle, this, &UWallJumpAbility::CheckAutoMantle, 0.05f, true);

		// 4. 튕겨나가는 방향에 맞춰 대시 몽타주(4섹션) 재활용
		if (AvatarCharacter->AirDashMontage)
		{
			FVector JumpDir = FinalVelocity.GetSafeNormal2D();
			FVector ForwardVec = AvatarCharacter->GetActorForwardVector();
			FVector RightVec = AvatarCharacter->GetActorRightVector();

			float JumpForwardDot = FVector::DotProduct(JumpDir, ForwardVec);
			float JumpRightDot = FVector::DotProduct(JumpDir, RightVec);

			FName SectionName = FName("Forward");
			if (JumpForwardDot > 0.5f) { SectionName = FName("Forward"); }
			else if (JumpForwardDot < -0.5f) { SectionName = FName("Backward"); }
			else if (JumpRightDot > 0.5f) { SectionName = FName("Right"); }
			else if (JumpRightDot < -0.5f) { SectionName = FName("Left"); }

			UAbilityTask_PlayMontageAndWait* JumpMontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
				this, NAME_None, AvatarCharacter->AirDashMontage, 1.0f, SectionName);
			JumpMontageTask->OnCompleted.AddDynamic(this, &UWallJumpAbility::EndAbilityByMontage);
			JumpMontageTask->OnInterrupted.AddDynamic(this, &UWallJumpAbility::EndAbilityByMontage);
			JumpMontageTask->OnCancelled.AddDynamic(this, &UWallJumpAbility::EndAbilityByMontage);
			JumpMontageTask->ReadyForActivation();

			UAbilityTask_WaitDelay* DelayTask = UAbilityTask_WaitDelay::WaitDelay(this, 0.35f);
			DelayTask->OnFinish.AddDynamic(this, &UWallJumpAbility::EndAbilityByMontage);
			DelayTask->ReadyForActivation();
		}
		else
		{
			EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
		}
	}
	else
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
	}
}

// 델리게이트 매핑용 헬퍼 함수
void UWallJumpAbility::EndAbilityByMontage()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UWallJumpAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	GetWorld()->GetTimerManager().ClearTimer(AutoMantleTimerHandle);

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UWallJumpAbility::CheckAutoMantle()
{
	ABaseCharacter* AvatarCharacter = Cast<ABaseCharacter>(CurrentActorInfo->AvatarActor.Get());
	UBaseMovementComponent* CMC = AvatarCharacter ? Cast<UBaseMovementComponent>(AvatarCharacter->GetCharacterMovement()) : nullptr;
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();

	if (AvatarCharacter && CMC && ASC)
	{
		FVector ForwardInput = CMC->GetLastInputVector();
		bool bIsPressingForward = FVector::DotProduct(ForwardInput.GetSafeNormal(), AvatarCharacter->GetActorForwardVector()) > 0.5f;

		if (bIsPressingForward && CMC->Velocity.Z > -200.0f)
		{
			FVector TargetLoc;
			float WallHeight;
			if (CMC->CheckVaulting(TargetLoc, WallHeight))
			{
				CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);

				FGameplayEventData Payload;
				Payload.EventMagnitude = WallHeight; // 벽 높이 전달

				FGameplayAbilityTargetData_LocationInfo* LocationData = new FGameplayAbilityTargetData_LocationInfo();
				LocationData->TargetLocation.LiteralTransform = FTransform(TargetLoc);
				LocationData->TargetLocation.LocationType = EGameplayAbilityTargetingLocationType::LiteralTransform;

				Payload.TargetData.Add(LocationData);

				FGameplayTag VaultTag = FGameplayTag::RequestGameplayTag(FName("Ability.Movement.Vault"));
				UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(AvatarCharacter, VaultTag, Payload);
			}
		}
	}
}