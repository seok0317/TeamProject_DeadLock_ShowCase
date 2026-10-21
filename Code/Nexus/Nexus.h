// 개인 구현 및 수정 범위 전시용. 독립 빌드 대상이 아닙니다.
// 분류: A — 본인 단독 구현
// 원본: Source/TeamProject_DL/GameCore/GameRule/Nexus.h
// 기준 커밋: db9a1720265f38f776d8c1ff003933a356ebeef3
// 기여 범위: 본인이 신규 작성하고 후속 수정한 파일. 일반 커밋·병합 combined diff·현재 blame에서 팀원의 기능 변경을 확인하지 못함.
// 원본 인코딩: utf-8-sig. UTF-8로 표시하고 BOM·줄바꿈을 정규화했습니다. 원문 주석/로그를 보존합니다.
// 원문 전체입니다. 팀원 지원 수정이 있는 B 파일은 위 기여 범위와 SourceManifest에 명시합니다.

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameCore/Character/AICharacter.h"
#include "Nexus.generated.h"

class USkeletalMeshComponent;
class USphereComponent;

UENUM(BlueprintType)
enum class ENexusPhase : uint8
{
	Phase1,
	Phase2
};

UCLASS()
class TEAMPROJECT_DL_API ANexus : public AAICharacter
{
	GENERATED_BODY()

public:
	ANexus(const FObjectInitializer& ObjectInitializer);

	// 씬 배치 시 팀 지정. 타워와 마찬가지로 충돌 및 무적 해제 식별에 사용.
	UPROPERTY(EditAnywhere, Category = "Nexus|State")
	int32 NexusTeamIdx = 0;

	// 1페이즈 피격용 스켈레탈 메쉬 (움직임은 없으나 스켈레탈 에셋으로 제공됨)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Nexus|Component")
	TObjectPtr<USkeletalMeshComponent> Phase1HitMesh;

	// UI 표시 범위를 위한 스피어 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Nexus|Component")
	TObjectPtr<USphereComponent> UITriggerSphere;

	// -------------------------------------------------------------------------
	// 무적 시스템
	// -------------------------------------------------------------------------
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category = "Nexus|State")
	bool bIsInvincible = true;

	// 최종 타워가 부서지면 호출하여 무적 해제
	UFUNCTION(BlueprintCallable, Category = "Nexus|State")
	void SetInvincible(bool bInvincible);

	// -------------------------------------------------------------------------
	// 페이즈 시스템
	// -------------------------------------------------------------------------
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Nexus|State")
	ENexusPhase CurrentPhase = ENexusPhase::Phase1;

	// 블루프린트에서 2페이즈로 넘어갈 때 (이펙트 스폰, 메쉬 교체, 애니메이션 재생, 체력 초기화 등) 처리할 이벤트
	UFUNCTION(BlueprintImplementableEvent, Category = "Nexus|Phase")
	void OnPhaseChanged(ENexusPhase NewPhase);

	// 1페이즈에서 2페이즈로 넘어가기 위한 함수
	UFUNCTION(BlueprintCallable, Category = "Nexus|Phase")
	void TransitionToPhase2();

	// 넥서스 파괴 시 서버에서 모든 클라이언트에게 뿌리는 멀티캐스트 RPC
	UFUNCTION(NetMulticast, Reliable, BlueprintCallable, Category = "Nexus|Phase")
	void MulticastNexusDestroyed();

	// 위 RPC가 도착하면 각 클라이언트가 로컬에서 폭발 연출 및 승패 UI를 띄우는 블루프린트 이벤트
	UFUNCTION(BlueprintImplementableEvent, Category = "Nexus|Phase")
	void OnNexusDestroyedEvent();

	// -------------------------------------------------------------------------
	// 스킬 이펙트 네트워킹 (서버 -> 모든 클라이언트)
	// -------------------------------------------------------------------------
	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlayAoECharging(class UNiagaraSystem* EffectSystem, FVector Scale, FVector Offset);

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastStopAoECharging();

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastPlayAoEExplosion(class UNiagaraSystem* EffectSystem, FVector Scale, FVector Offset);

	UPROPERTY()
	TObjectPtr<class UNiagaraComponent> ActiveChargingNiagara;

	// -------------------------------------------------------------------------
	// 오버라이드 기능
	// -------------------------------------------------------------------------
	// 데미지 파이프라인에서 무적 방어 (GAS 데미지 이펙트가 TakeDamage를 통하지 않을 경우 BP에서 별도 차단 필요)
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	virtual void ApplyTeamCollision(int32 InTeamIdx) override;

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 플레이어 접근 판별
	UFUNCTION()
	void OnUITriggerBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnUITriggerEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// UI 업데이트 알림 이벤트 (플레이어의 블루프린트에서 체력바 갱신에 활용)
	// bShow가 true면 UI 띄우기, false면 숨기기
	UFUNCTION(BlueprintImplementableEvent, Category = "Nexus|UI")
	void UpdateNexusUI(AActor* PlayerActor, bool bShow);

	// 체력 0 도달 시 호출 (HealthComponent 사망 델리게이트)
	UFUNCTION()
	void OnNexusDeath(AActor* DeadActor, AActor* InInstigator);

	// 체력 변화 시 페이즈 전환 검사용 (절반 이하 체크)
	UFUNCTION()
	void OnNexusHealthChanged(float OldValue, float NewValue, float MaxHealth);
};
