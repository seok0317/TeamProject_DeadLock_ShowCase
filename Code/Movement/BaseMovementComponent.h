// 개인 구현 및 수정 범위 전시용. 독립 빌드 대상이 아닙니다.
// 분류: A — 본인 단독 구현
// 원본: Source/TeamProject_DL/MovementComponent/BaseMovementComponent.h
// 기준 커밋: db9a1720265f38f776d8c1ff003933a356ebeef3
// 기여 범위: 본인이 신규 작성하고 후속 수정한 파일. 일반 커밋·병합 combined diff·현재 blame에서 팀원의 기능 변경을 확인하지 못함.
// 원본 인코딩: utf-8-sig. UTF-8로 표시하고 BOM·줄바꿈을 정규화했습니다. 원문 주석/로그를 보존합니다.
// 원문 전체입니다. 팀원 지원 수정이 있는 B 파일은 위 기여 범위와 SourceManifest에 명시합니다.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BaseMovementComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWallClingStateChanged, bool, bIsClinging);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSlideStateChanged, bool, bIsSliding);

UCLASS()
class TEAMPROJECT_DL_API UBaseMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void UpdateCharacterStateBeforeMovement(float DeltaSeconds) override;
	virtual void UpdateCharacterStateAfterMovement(float DeltaSeconds) override;
	virtual void UpdateFromCompressedFlags(uint8 Flags) override;
	virtual void CalcVelocity(float DeltaTime, float Friction, bool bFluid, float BrakingDeceleration) override;

	class FSavedMove_BaseMovement : public FSavedMove_Character
	{
	public:
		typedef FSavedMove_Character Super;

		// 클라이언트가 서버로 보낼 플래그 변수
		uint8 bSavedIsSliding : 1;
		uint8 bSavedIsWallClinging : 1;

		virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const override;
		virtual void Clear() override;
		virtual uint8 GetCompressedFlags() const override;
		virtual void SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, class FNetworkPredictionData_Client_Character& ClientData) override;
		virtual void PrepMoveFor(ACharacter* C) override;
	};

	class FNetworkPredictionData_Client_BaseMovement : public FNetworkPredictionData_Client_Character
	{
	public:
		FNetworkPredictionData_Client_BaseMovement(const UCharacterMovementComponent& ClientMovement);
		typedef FNetworkPredictionData_Client_Character Super;
		virtual FSavedMovePtr AllocateNewMove() override;
	};

	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;

	UFUNCTION(BlueprintCallable, Category = "Movement|Slide")
	bool CanSlide() const;

	void StartSlide();
	void StopSlide();

	bool bIsSliding = false;

	void PrepareAirDash(FVector DashDirection);
	void ResetGravity();
	FVector GetCachedPreDashVelocity() const { return CachedPreDashVelocity; }

	bool IsWallClinging() const { return bIsWallClinging; }
	FVector GetWallNormal() const { return CachedWallNormal; }
	void ReleaseWallCling();

	UPROPERTY(BlueprintAssignable, Category = "Movement|Events")
	FOnWallClingStateChanged OnWallClingStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Movement|Events")
	FOnSlideStateChanged OnSlideStateChanged;

	bool bDisableWallCling = false;

public:

	// 벽을 넘을 수 있는지(오를 수 있는지) 검사하고, 목표 위치와 벽의 높이를 반환합
	UFUNCTION(BlueprintCallable, Category = "Movement|Vault")
	bool CheckVaulting(FVector& OutTargetLocation, float& OutWallHeight);

protected:
	virtual void BeginPlay() override;
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;

	UPROPERTY()
	class ABaseCharacter* CachedBaseChar = nullptr;

	// 슬라이딩 발동 최소 조건
	UPROPERTY(EditDefaultsOnly, Category = "Movement|Slide")
	float MinSlideSpeed = 400.0f;

	// 마찰력 설정
	UPROPERTY(EditDefaultsOnly, Category = "Movement|Slide")
	float SlideFriction = 0.0f; // 슬라이딩 중 기본 마찰력 

	// 평지 슬라이딩 설정
	UPROPERTY(EditDefaultsOnly, Category = "Movement|Slide")
	float FlatSlideBraking = 1200.0f;

	// 경사로(내리막길) 슬라이딩 설정
	UPROPERTY(EditDefaultsOnly, Category = "Movement|Slide")
	float SlopeSlideBraking = 100.0f; 

	UPROPERTY(EditDefaultsOnly, Category = "Movement|Slide")
	float SlopeGravity = 3000.0f;

	// 최고속도
	UPROPERTY(EditDefaultsOnly, Category = "Movement|Slide")
	float MaxSlopeSlideSpeed = 1800.0f; 

	// 동적 브레이크 비율
	UPROPERTY(EditDefaultsOnly, Category = "Movement|Slide")
	float HighSpeedBrakingMultiplier = 2.0f;

	// 동적 브레이크가 적용되는 초과 속도 기준점
	UPROPERTY(EditDefaultsOnly, Category = "Movement|Slide")
	float MinSpeedForHighBraking = 500.0f;

	// 선회력 
	UPROPERTY(EditDefaultsOnly, Category = "Movement|Slide")
	float SlideSteeringRate = 3.0f;

	/*========================= 벽넘기 ===========================*/

	UPROPERTY(EditDefaultsOnly, Category = "Movement|Vault")
	float VaultForwardTraceDistance = 80.0f; // 앞으로 쏘는 길이

	UPROPERTY(EditDefaultsOnly, Category = "Movement|Vault")
	float VaultUpwardTraceHeight = 350.0f; // 캐릭터 키의 1.5배 (기본 반홀 높이 약 90 * 1.5)

	UPROPERTY(EditDefaultsOnly, Category = "Movement|Vault")
	float VaultMinHeight = 30.0f; // 최소 파쿠르 가능 높이

	UPROPERTY(EditDefaultsOnly, Category = "Movement|Vault")
	float VaultDepthOffsetMargin = 15.0f; // 착지 시 벽 안쪽으로 들어갈 여유 거리 (Radius + Margin)

	UPROPERTY(EditDefaultsOnly, Category = "Movement|Vault")
	float VaultZOffsetMargin = 2.0f; // 착지 높이 미세 보정 (땅에 안착하기 위함)

private:
	void UpdateWallCling(float DeltaTime);

	float DefaultGravityScale = 1.0f;
	FVector CachedPreDashVelocity = FVector::ZeroVector;

	bool bIsWallClinging = false;
	FVector CachedWallNormal = FVector::ZeroVector;

	// 벽 감지 거리
	float WallClingTraceDistance = 70.0f;

};