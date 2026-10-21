// 개인 구현 및 수정 범위 전시용. 독립 빌드 대상이 아닙니다.
// 분류: A — 본인 단독 구현
// 원본: Source/TeamProject_DL/MovementComponent/Ability/GroundRollAbility.h
// 기준 커밋: db9a1720265f38f776d8c1ff003933a356ebeef3
// 기여 범위: 본인이 신규 작성하고 후속 수정한 파일. 일반 커밋·병합 combined diff·현재 blame에서 팀원의 기능 변경을 확인하지 못함.
// 원본 인코딩: utf-8-sig. UTF-8로 표시하고 BOM·줄바꿈을 정규화했습니다. 원문 주석/로그를 보존합니다.
// 원문 전체입니다. 팀원 지원 수정이 있는 B 파일은 위 기여 범위와 SourceManifest에 명시합니다.

// --- GroundRollAbility.h ---
#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GroundRollAbility.generated.h"

UCLASS()
class TEAMPROJECT_DL_API UGroundRollAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGroundRollAbility();

protected:
	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		bool bReplicateEndAbility,
		bool bWasCancelled) override;

	UFUNCTION()
	void OnRollFinished();

	UFUNCTION()
	void OnRollDurationFinished();

	UFUNCTION()
	void RestoreCameraLag();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Roll")
	float RollSpeed = 1000.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Roll")
	float RollDuration = 1.23f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Roll")
	float JumpStartTiming = 0.54f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Roll")
	float JumpEndTiming = 0.8f;

	FVector CachedRollDir;
	bool bIsEnding = false;

	bool bCachedCameraLagEnabled = true;
	float CachedCameraLagSpeed = 15.0f;
	bool bCameraRestored = false;

	FTimerHandle RollJumpWindowTimerHandle;
	FTimerHandle RollJumpWindowEndTimerHandle;

	void OpenRollJumpWindow();
	void CloseRollJumpWindow();
};