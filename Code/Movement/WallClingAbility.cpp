// 개인 구현 및 수정 범위 전시용. 독립 빌드 대상이 아닙니다.
// 분류: A — 본인 단독 구현
// 원본: Source/TeamProject_DL/MovementComponent/Ability/WallClingAbility.cpp
// 기준 커밋: db9a1720265f38f776d8c1ff003933a356ebeef3
// 기여 범위: 본인이 신규 작성하고 후속 수정한 파일. 일반 커밋·병합 combined diff·현재 blame에서 팀원의 기능 변경을 확인하지 못함.
// 원본 인코딩: utf-8-sig. UTF-8로 표시하고 BOM·줄바꿈을 정규화했습니다. 원문 주석/로그를 보존합니다.
// 원문 전체입니다. 팀원 지원 수정이 있는 B 파일은 위 기여 범위와 SourceManifest에 명시합니다.

#include "../../MovementComponent/Ability/WallClingAbility.h" 
#include "GameFramework/Character.h"
#include "../../GameCore/Character/BaseCharacter.h"
#include "../../MovementComponent/BaseMovementComponent.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"

UWallClingAbility::UWallClingAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Movement.WallCling")));
}

void UWallClingAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	bIsEnding = false;

	ABaseCharacter* AvatarCharacter = Cast<ABaseCharacter>(ActorInfo->AvatarActor.Get());
	UBaseMovementComponent* CMC = AvatarCharacter ? Cast<UBaseMovementComponent>(AvatarCharacter->GetCharacterMovement()) : nullptr;

	if (AvatarCharacter && CMC && AvatarCharacter->WallClingMontage)
	{
		// 1. 벽 방향(Wall Normal)과 내 정면/우측 벡터를 비교(내적)
		FVector WallNormal = CMC->GetWallNormal();
		FVector ForwardVec = AvatarCharacter->GetActorForwardVector();
		FVector RightVec = AvatarCharacter->GetActorRightVector();

		float ForwardDot = FVector::DotProduct(WallNormal, ForwardVec);
		float RightDot = FVector::DotProduct(WallNormal, RightVec);

		// 2. 내적 값으로 섹션 이름 결정
		FName SectionName = NAME_None;
		UAnimMontage* ActiveMontage = AvatarCharacter->WallClingMontage;
		if (ForwardDot < -0.5f) { SectionName = FName("Front"); }
		else if (ForwardDot > 0.5f){ ActiveMontage = nullptr; }
		else if (RightDot > 0.5f) { SectionName = FName("Left"); }
		else { SectionName = FName("Right"); }

		// 3. 몽타주 재생 
		if (ActiveMontage)
		{
			UAbilityTask_PlayMontageAndWait* MontageTask = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
				this, NAME_None, ActiveMontage, 1.0f, SectionName);

			MontageTask->OnCompleted.AddDynamic(this, &UWallClingAbility::OnMontageFinished);
			MontageTask->OnInterrupted.AddDynamic(this, &UWallClingAbility::OnMontageFinished);
			MontageTask->OnCancelled.AddDynamic(this, &UWallClingAbility::OnMontageFinished);
			MontageTask->ReadyForActivation();
		}
	}
	else
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
	}
}

void UWallClingAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	if (bIsEnding) return;
	bIsEnding = true;

	ABaseCharacter* AvatarCharacter = Cast<ABaseCharacter>(ActorInfo->AvatarActor.Get());

	// 어빌리티가 종료될 때 몽타주를 끔
	if (AvatarCharacter && AvatarCharacter->WallClingMontage)
	{
		AvatarCharacter->StopAnimMontage(AvatarCharacter->WallClingMontage);
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UWallClingAbility::OnMontageFinished()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}