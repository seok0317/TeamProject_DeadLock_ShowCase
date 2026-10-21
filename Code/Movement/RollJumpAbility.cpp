// 개인 구현 및 수정 범위 전시용. 독립 빌드 대상이 아닙니다.
// 분류: A — 본인 단독 구현
// 원본: Source/TeamProject_DL/MovementComponent/Ability/RollJumpAbility.cpp
// 기준 커밋: db9a1720265f38f776d8c1ff003933a356ebeef3
// 기여 범위: 본인이 신규 작성하고 후속 수정한 파일. 일반 커밋·병합 combined diff·현재 blame에서 팀원의 기능 변경을 확인하지 못함.
// 원본 인코딩: utf-8-sig. UTF-8로 표시하고 BOM·줄바꿈을 정규화했습니다. 원문 주석/로그를 보존합니다.
// 원문 전체입니다. 팀원 지원 수정이 있는 B 파일은 위 기여 범위와 SourceManifest에 명시합니다.

#include "../../MovementComponent/Ability/RollJumpAbility.h" 
#include "GameFramework/Character.h"
#include "../../GameCore/Character/BaseCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AbilitySystemComponent.h"

URollJumpAbility::URollJumpAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	// 어빌리티 태그 등록
	AbilityTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Movement.RollJump")));
}

void URollJumpAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	ABaseCharacter* AvatarCharacter = Cast<ABaseCharacter>(ActorInfo->AvatarActor.Get());
	if (AvatarCharacter)
	{
		FVector ForwardDir = AvatarCharacter->GetActorForwardVector();

		FVector LaunchVelocity = ForwardDir * RollJumpForwardBoost;
		LaunchVelocity.Z = RollJumpVerticalForce;

		// true, true = 기존 물리 속도를 무시하고 이 속도로 강제 덮어씌움
		AvatarCharacter->LaunchCharacter(LaunchVelocity, true, true);
	}

	EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
}