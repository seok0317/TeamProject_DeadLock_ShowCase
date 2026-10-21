// 개인 구현 및 수정 범위 전시용. 독립 빌드 대상이 아닙니다.
// 분류: A — 본인 단독 구현
// 원본: Source/TeamProject_DL/MovementComponent/Ability/VaultAbility.h
// 기준 커밋: db9a1720265f38f776d8c1ff003933a356ebeef3
// 기여 범위: 본인이 신규 작성하고 후속 수정한 파일. 일반 커밋·병합 combined diff·현재 blame에서 팀원의 기능 변경을 확인하지 못함.
// 원본 인코딩: utf-8-sig. UTF-8로 표시하고 BOM·줄바꿈을 정규화했습니다. 원문 주석/로그를 보존합니다.
// 원문 전체입니다. 팀원 지원 수정이 있는 B 파일은 위 기여 범위와 SourceManifest에 명시합니다.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "VaultAbility.generated.h"

USTRUCT(BlueprintType)
struct FVaultAnimThreshold
{
	GENERATED_BODY()

	// 이 높이 '이하'일 때 해당 섹션이 재생됨
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Vault")
	float MaxHeight = 0.0f;

	// 재생할 몽타주 섹션 이름
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Vault")
	FName SectionName = NAME_None;
};

UCLASS()
class TEAMPROJECT_DL_API UVaultAbility : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UVaultAbility();

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
	void VaultTick();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Vault")
	UCurveFloat* VaultTimeMappingCurve;

	// 루트 모션 이동이 완료되었을 때 호출
	UFUNCTION()
	void OnVaultMovementFinished();

	// 올라가는 데 걸리는 시간
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Vault")
	float VaultDuration = 0.4f;

	// 벽 높이 배열
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Vault")
	TArray<FVaultAnimThreshold> VaultThresholds;

private:
	bool bIsEnding = false;

	FVector VaultStartLocation;
	FVector VaultTargetLocation;
	float VaultElapsedTime;
	float ActualVaultDuration;
	FTimerHandle VaultTimerHandle;
};