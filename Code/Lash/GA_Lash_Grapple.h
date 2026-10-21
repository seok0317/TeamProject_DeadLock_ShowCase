// 개인 구현 및 수정 범위 전시용. 독립 빌드 대상이 아닙니다.
// 분류: A — 본인 단독 구현
// 원본: Source/TeamProject_DL/Lash/GA_Lash_Grapple.h
// 기준 커밋: db9a1720265f38f776d8c1ff003933a356ebeef3
// 기여 범위: 본인이 신규 작성하고 후속 수정한 파일. 일반 커밋·병합 combined diff·현재 blame에서 팀원의 기능 변경을 확인하지 못함.
// 원본 인코딩: utf-8-sig. UTF-8로 표시하고 BOM·줄바꿈을 정규화했습니다. 원문 주석/로그를 보존합니다.
// 원문 전체입니다. 팀원 지원 수정이 있는 B 파일은 위 기여 범위와 SourceManifest에 명시합니다.

#pragma once

#include "CoreMinimal.h"
#include "Public/GA_SkillBase.h" 
#include "GA_Lash_Grapple.generated.h"

class UNiagaraSystem;
class UNiagaraComponent;
class UAbilityTask_ApplyRootMotionConstantForce;
class UGameplayEffect;

UCLASS()
class TEAMPROJECT_DL_API UGA_Lash_Grapple : public UGA_SkillBase
{
	GENERATED_BODY()

public:
	UGA_Lash_Grapple();

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:

	// 물리 및 비주얼 처리
	virtual void OnCastStarted_Implementation() override;

	UPROPERTY(EditDefaultsOnly, Category = "Skill|Lash")
	TSubclassOf<UGameplayEffect> GrappleFlyingEffectClass;

	// 비행 태스크 완료 시 콜백
	UFUNCTION()
	void OnGrappleFinished();

	// 2초 로프 수명 만료 시 호출
	UFUNCTION()
	void CleanupRope();

protected:
	//  래쉬 그래플 설정값 

	// 발사 위력 
	UPROPERTY(EditDefaultsOnly, Category = "Skill|Lash")
	float LaunchStrength = 2800.0f;

	// 추진력을 가할 시간
	UPROPERTY(EditDefaultsOnly, Category = "Skill|Lash")
	float LaunchDuration = 0.5f;

	UFUNCTION()
	void OnMovementModeChanged(ACharacter* Character, EMovementMode PrevMovementMode, uint8 PreviousCustomMode);


};