// 개인 구현 및 수정 범위 전시용. 독립 빌드 대상이 아닙니다.
// 분류: A — 본인 단독 구현
// 원본: Source/TeamProject_DL/GameCore/Combat/Ability/GA_Nexus_AoE.h
// 기준 커밋: db9a1720265f38f776d8c1ff003933a356ebeef3
// 기여 범위: 본인이 신규 작성하고 후속 수정한 파일. 일반 커밋·병합 combined diff·현재 blame에서 팀원의 기능 변경을 확인하지 못함.
// 원본 인코딩: utf-8-sig. UTF-8로 표시하고 BOM·줄바꿈을 정규화했습니다. 원문 주석/로그를 보존합니다.
// 원문 전체입니다. 팀원 지원 수정이 있는 B 파일은 위 기여 범위와 SourceManifest에 명시합니다.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_Nexus_AoE.generated.h"

class UNiagaraSystem;
class UNiagaraComponent;
class UAnimMontage;
class UGameplayEffect;

UCLASS()
class TEAMPROJECT_DL_API UGA_Nexus_AoE : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_Nexus_AoE();

protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageInterrupted();

	UFUNCTION()
	void OnExplosionEvent(FGameplayEventData Payload);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Nexus|AoE")
	TObjectPtr<UAnimMontage> AttackMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Nexus|AoE")
	TObjectPtr<UNiagaraSystem> ChargingNiagara;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Nexus|AoE")
	TObjectPtr<UNiagaraSystem> ExplosionNiagara;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Nexus|AoE")
	TSubclassOf<UGameplayEffect> DamageEffectClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Nexus|AoE")
	FVector EffectScale = FVector(1.f, 1.f, 1.f);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Nexus|AoE")
	FVector EffectOffset = FVector::ZeroVector;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Nexus|AoE")
	float ChargingDuration = 2.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Nexus|AoE")
	float DamageCoefficient = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Nexus|AoE")
	float BaseDamage = 100.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Nexus|AoE")
	float DamageRadius = 1000.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Nexus|AoE")
	FGameplayTag ExplodeEventTag;

private:
	UPROPERTY()
	TObjectPtr<UNiagaraComponent> SpawnedChargingNiagara;
};
