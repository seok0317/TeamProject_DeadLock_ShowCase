// 개인 구현 및 수정 범위 전시용. 독립 빌드 대상이 아닙니다.
// 분류: A — 본인 단독 구현
// 원본: Source/TeamProject_DL/Lash/GA_Lash_Flog.h
// 기준 커밋: db9a1720265f38f776d8c1ff003933a356ebeef3
// 기여 범위: 본인이 신규 작성하고 후속 수정한 파일. 일반 커밋·병합 combined diff·현재 blame에서 팀원의 기능 변경을 확인하지 못함.
// 원본 인코딩: utf-8-sig. UTF-8로 표시하고 BOM·줄바꿈을 정규화했습니다. 원문 주석/로그를 보존합니다.
// 원문 전체입니다. 팀원 지원 수정이 있는 B 파일은 위 기여 범위와 SourceManifest에 명시합니다.

#pragma once

#include "CoreMinimal.h"
#include "Public/GA_SkillBase.h"
#include "Components/PostProcessComponent.h"
#include "GA_Lash_Flog.generated.h"

UCLASS()
class TEAMPROJECT_DL_API UGA_Lash_Flog : public UGA_SkillBase
{
	GENERATED_BODY()

public:
	UGA_Lash_Flog();

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	virtual void OnCastStarted_Implementation() override;
	virtual void OnSkillEvent_Implementation(FGameplayTag EventTag, FGameplayEventData Payload) override;

	// 조작 차단 태그만 해제하는 함수
	UFUNCTION()
	void TimerUnlockControls();

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Skill|Lash")
	float BaseDamage = 100.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Skill|Lash")
	float SpiritCoefficient = 0.8f;

	UPROPERTY(EditDefaultsOnly, Category = "Skill|Lash")
	float HealMultiplier = 0.8f;

	UPROPERTY(EditDefaultsOnly, Category = "Skill|Lash")
	TSubclassOf<class UGameplayEffect> HealEffectClass;

	UPROPERTY(EditDefaultsOnly, Category = "Skill|Lash")
	float SkillLockDuration = 0.1f;

	UPROPERTY(EditDefaultsOnly, Category = "Skill|Lash")
	TSubclassOf<UGameplayEffect> FlogEffectClass;

private:
	FTimerHandle AbilityEndTimerHandle;
};