// 개인 구현 및 수정 범위 전시용. 독립 빌드 대상이 아닙니다.
// 분류: A — 본인 단독 구현
// 원본: Source/TeamProject_DL/Lash/GA_Lash_DeathSlam.h
// 기준 커밋: db9a1720265f38f776d8c1ff003933a356ebeef3
// 기여 범위: 본인이 신규 작성하고 후속 수정한 파일. 일반 커밋·병합 combined diff·현재 blame에서 팀원의 기능 변경을 확인하지 못함.
// 원본 인코딩: utf-8-sig. UTF-8로 표시하고 BOM·줄바꿈을 정규화했습니다. 원문 주석/로그를 보존합니다.
// 원문 전체입니다. 팀원 지원 수정이 있는 B 파일은 위 기여 범위와 SourceManifest에 명시합니다.

#pragma once

#include "CoreMinimal.h"
#include "Public/GA_SkillBase.h" 
#include "GA_Lash_DeathSlam.generated.h"

class UWidgetComponent;
class UNiagaraComponent;
class ULashLockonWidget;
class UGameplayEffect;

// 개별 적의 락온 상태와 영수증(Handle) 관리 구조체
USTRUCT(BlueprintType)
struct FLashLockonTarget
{
	GENERATED_BODY()

	UPROPERTY()
	TObjectPtr<ACharacter> TargetCharacter = nullptr;

	float GazeDuration = 0.0f;
	bool bIsLockedOn = false;

	UPROPERTY()
	TObjectPtr<UWidgetComponent> LockonWidgetComp = nullptr;
	UPROPERTY()
	TObjectPtr<UNiagaraComponent> PullRopeFX = nullptr;

	FActiveGameplayEffectHandle SuspendStunGEHandle;
	ECollisionResponse OriginalPawnCollisionResponse = ECR_Block;
	EMovementMode OriginalMovementMode = MOVE_Walking;
};

UCLASS()
class TEAMPROJECT_DL_API UGA_Lash_DeathSlam : public UGA_SkillBase
{
	GENERATED_BODY()

public:
	UGA_Lash_DeathSlam();

	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled) override;

protected:
	virtual void OnCastStarted_Implementation() override;
	virtual void OnSkillEvent_Implementation(FGameplayTag EventTag, FGameplayEventData Payload) override;

	// [Phase 1] 1초 응시 감시 레이더
	UFUNCTION()
	void UpdateLockonTargets();

	// [Phase 1 -> 2] 좌클릭 시 확정 및 데이터 서버 전송
	UFUNCTION()
	void OnConfirmPressed(float TimeWaited);

	// [Phase 1] 5초 타임아웃
	UFUNCTION()
	void OnScopeTimeout();

	// [Phase 2] 서버/클라가 타겟 데이터를 받고 래쉬를 띄우는 함수
	UFUNCTION()
	void OnPhase1DataReceived(const FGameplayAbilityTargetDataHandle& Data, FGameplayTag ApplicationTag);

	UFUNCTION()
	void ExecuteServerLaunch();

	// [Phase 2] 래쉬 부양 종료
	UFUNCTION()
	void OnAscendFinished();

	// [Phase 2] 공중에서 적 홀딩 + 로프 위치 실시간 트래킹 + 바닥 범위 인디케이터 이동
	UFUNCTION()
	void UpdateHoldingAndVisualFX();

	// [Phase 3] 클라 에임 바닥 좌표 계산 후 다이렉트 서버 전송 (RPC)
	UFUNCTION()
	void CalculateAndSendSlamTarget();

	UFUNCTION(Server, Reliable)
	void Server_SendSlamTargetLoc(FVector TargetLoc);

	// [Phase 3] 내리찍기 몽타주 시작 (방향 회전)
	void ProcessSlamDive(FVector TargetLoc);

	// [Phase 3-B] 실제 손을 내지르는 타이밍에 적들을 바닥으로 패대기
	UFUNCTION()
	void LaunchEnemiesToGround();

	// [Phase 3-C] 바닥에 충돌했는지 실시간 추적 (+ 로프 위치 트래킹)
	UFUNCTION()
	void CheckSlamImpact();

	// [Phase 4] 바닥 폭발 및 스턴 해제
	UFUNCTION()
	void ExecuteSlamImpact();	
	
	UFUNCTION(Server, Reliable)
	void Server_CancelSkill();

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Skill|DeathSlam|Montage")
	TObjectPtr<class UAnimMontage> GroundScanMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Skill|DeathSlam|Montage")
	TObjectPtr<class UAnimMontage> AirborneScanMontage;

	UPROPERTY(EditDefaultsOnly, Category = "Skill|DeathSlam")
	TSubclassOf<ULashLockonWidget> LockonWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Skill|DeathSlam")
	float RequiredGazeTime = 1.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Skill|DeathSlam")
	TSubclassOf<UGameplayEffect> SuspendStunEffectClass;

	// --- [FX 카테고리] ---
	UPROPERTY(EditDefaultsOnly, Category = "Skill|DeathSlam|FX")
	TObjectPtr<class UNiagaraSystem> PullRopeFXClass;

	//바닥에 찍힐 때 발생하는 충격파 폭발 이펙트 (태그 방식으로 교체)
	UPROPERTY(EditDefaultsOnly, Category = "Skill|DeathSlam|FX")
	FGameplayTag SlamImpactCueTag;

	// 체공 중 내 에임이 가리키는 바닥 착지 범위를 표시해 주는 장판 인디케이터 이펙트
	UPROPERTY(EditDefaultsOnly, Category = "Skill|DeathSlam|FX")
	TObjectPtr<class UNiagaraSystem> TargetIndicatorFXClass;

	UPROPERTY(EditDefaultsOnly, Category = "Skill|DeathSlam")
	float AscendHeight = 350.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Skill|DeathSlam|Holding")
	float HoldForwardDistance = 250.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Skill|DeathSlam|Holding")
	float HoldHeightOffset = -50.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Skill|DeathSlam|Timing")
	float SlamDiveDelay = 0.35f;

	UPROPERTY(EditDefaultsOnly, Category = "Skill|DeathSlam")
	float SlamRadius = 500.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Skill|DeathSlam")
	float BaseDamage = 200.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Skill|DeathSlam")
	float SpiritCoefficient = 1.5f;

private:
	FTimerHandle LockonTimerHandle;
	FTimerHandle HoverTimerHandle;
	FTimerHandle GroundSlamTimerHandle;
	FTimerHandle ScopeTimeoutTimerHandle;
	FTimerHandle HoldEnemiesTimerHandle;
	FTimerHandle SlamDiveTimerHandle;
	FTimerHandle ServerLaunchTimerHandle;

	FVector FinalTargetLocation;
	bool bHasSlammed = false;
	bool bHasLaunchedDown = false;
	float SlamFlightTime = 0.0f;

	UPROPERTY()
	TObjectPtr<UNiagaraComponent> TargetIndicatorFXComp = nullptr;

	UPROPERTY()
	TArray<FLashLockonTarget> TrackedTargets;

	UPROPERTY()
	TArray<FLashLockonTarget> ConfirmedTargets;
};