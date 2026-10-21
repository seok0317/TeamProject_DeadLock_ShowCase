// 개인 구현 및 수정 범위 전시용. 독립 빌드 대상이 아닙니다.
// 분류: A — 본인 단독 구현
// 원본: Source/TeamProject_DL/MovementComponent/Ability/SlideAbility.cpp
// 기준 커밋: db9a1720265f38f776d8c1ff003933a356ebeef3
// 기여 범위: 본인이 신규 작성하고 후속 수정한 파일. 일반 커밋·병합 combined diff·현재 blame에서 팀원의 기능 변경을 확인하지 못함.
// 원본 인코딩: utf-8-sig. UTF-8로 표시하고 BOM·줄바꿈을 정규화했습니다. 원문 주석/로그를 보존합니다.
// 원문 전체입니다. 팀원 지원 수정이 있는 B 파일은 위 기여 범위와 SourceManifest에 명시합니다.

#include "../../MovementComponent/Ability/SlideAbility.h" 
#include "GameFramework/Character.h"
#include "../../GameCore/Character/BaseCharacter.h"
#include "../../MovementComponent/BaseMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

USlideAbility::USlideAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Movement.Slide")));
	ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("State.Movement.Slide")));

	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("State.Movement.AirDash")));
	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("State.Movement.Rolling")));
}

void USlideAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	bIsEnding = false;

	ABaseCharacter* AvatarCharacter = Cast<ABaseCharacter>(ActorInfo->AvatarActor.Get());
	UBaseMovementComponent* CMC = AvatarCharacter ? Cast<UBaseMovementComponent>(AvatarCharacter->GetCharacterMovement()) : nullptr;

	if (AvatarCharacter && CMC && CMC->CanSlide())
	{
		// 물리 슬라이딩 시작
		CMC->StartSlide();

		// 몽타주 재생 (Start 섹션부터 시작 -> 자동으로 Loop 무한 반복)
		if (AvatarCharacter->SlideMontage)
		{
			UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
				this, NAME_None, AvatarCharacter->SlideMontage, 1.0f, FName("Start"));

			MontageTask->OnCompleted.AddDynamic(this, &USlideAbility::OnMontageFinished);
			MontageTask->OnInterrupted.AddDynamic(this, &USlideAbility::OnMontageFinished);
			MontageTask->OnCancelled.AddDynamic(this, &USlideAbility::OnMontageFinished);
			MontageTask->ReadyForActivation();
		}
	}
	else
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
	}
}

void USlideAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (bIsEnding) return;
	bIsEnding = true;

	ABaseCharacter* AvatarCharacter = Cast<ABaseCharacter>(ActorInfo->AvatarActor.Get());
	UBaseMovementComponent* CMC = AvatarCharacter ? Cast<UBaseMovementComponent>(AvatarCharacter->GetCharacterMovement()) : nullptr;

	if (CMC)
	{
		// 마찰력 복구
		CMC->StopSlide();
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	// 일어나는 몽타주
	if (AvatarCharacter && AvatarCharacter->SlideMontage)
	{
		AvatarCharacter->PlayAnimMontage(AvatarCharacter->SlideMontage, 1.0f, FName("Getup"));
	}
}

void USlideAbility::OnMontageFinished()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}