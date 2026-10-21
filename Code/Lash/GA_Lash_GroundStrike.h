// 개인 구현 및 수정 범위 전시용. 독립 빌드 대상이 아닙니다.
// 분류: A — 본인 단독 구현
// 원본: Source/TeamProject_DL/Lash/GA_Lash_GroundStrike.h
// 기준 커밋: db9a1720265f38f776d8c1ff003933a356ebeef3
// 기여 범위: 본인이 신규 작성하고 후속 수정한 파일. 일반 커밋·병합 combined diff·현재 blame에서 팀원의 기능 변경을 확인하지 못함.
// 원본 인코딩: utf-8-sig. UTF-8로 표시하고 BOM·줄바꿈을 정규화했습니다. 원문 주석/로그를 보존합니다.
// 원문 전체입니다. 팀원 지원 수정이 있는 B 파일은 위 기여 범위와 SourceManifest에 명시합니다.

#pragma once

#include "Abilities/Tasks/AbilityTask_ApplyRootMotionMoveToForce.h"
#include "CoreMinimal.h"
#include "Public/GA_SkillBase.h" 
#include "GA_Lash_GroundStrike.generated.h"

class UAnimMontage;
class UGameplayEffect;

UCLASS()
class TEAMPROJECT_DL_API UGA_Lash_GroundStrike : public UGA_SkillBase
{
	GENERATED_BODY()

public:
	UGA_Lash_GroundStrike();

protected:
	// 시전 몽타주 시작 시 호출 
	virtual void OnCastStarted_Implementation() override;

	// 애니메이션 노티파이(GameplayEvent) 수신 시 타격 처리
	virtual void OnSkillEvent_Implementation(FGameplayTag EventTag, FGameplayEventData Payload) override;


protected:

	// 공중에서 시전할 때 사용할 강하 몽타주
	UPROPERTY(EditDefaultsOnly, Category = "Skill|Lash")
	TObjectPtr<UAnimMontage> AirborneCastMontage;

	// 타격 판정 반경
	UPROPERTY(EditAnywhere, Category = "Skill|Lash")
	float StrikeRadius = 350.0f;

	// 공중 강하 시 날아가는 속도 
	UPROPERTY(EditAnywhere, Category = "Skill|Lash")
	float DiveSpeed = 2500.0f;

	// 폭발 데미지를 적용할 GameplayEffect 클래스
	UPROPERTY(EditDefaultsOnly, Category = "Skill|Lash")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	// 기본 데미지 수치
	UPROPERTY(EditAnywhere, Category = "Skill|Lash")
	float BaseDamage = 150.0f;

	UPROPERTY(EditAnywhere, Category = "Skill|Lash")
	float SpiritCoefficient = 1.0f;

	// 부채꼴의 각도 
	UPROPERTY(EditDefaultsOnly, Category = "Skill|Lash", meta = (ClampMin = "0.0", ClampMax = "360.0"))
	float ConeAngleDegrees = 90.0f;

	UPROPERTY()
	TObjectPtr<UAbilityTask_ApplyRootMotionMoveToForce> DiveTask;

	// 다이브가 목적지에 도착했을 때 호출될 델리게이트
	UFUNCTION()
	void OnDiveCompleted();

	UFUNCTION()
	void OnMovementModeChanged(ACharacter* Character, EMovementMode PrevMovementMode, uint8 PreviousCustomMode);

	UPROPERTY(EditDefaultsOnly, Category = "Skill|Lash|VFX")
	FGameplayTag ImpactCueTag;

private:
	// 타격 대상 필터링 함수
	void ExecuteStrikeDamage(const FVector& ImpactLocation);
};