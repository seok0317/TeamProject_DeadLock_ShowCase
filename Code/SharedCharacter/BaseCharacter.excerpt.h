// 개인 구현 및 수정 범위 전시용. 독립 빌드 대상이 아닙니다.
// 분류: C — 팀원과 기능 공동 관여
// 원본: Source/TeamProject_DL/GameCore/Character/BaseCharacter.h
// 기준 커밋: db9a1720265f38f776d8c1ff003933a356ebeef3
// 기여 범위: 공통 캐릭터는 팀 공동 파일. 본인의 이동·체력바·HUD·패시브 관련 선언만 발췌. 총격·성장·사망 시스템 전체를 본인 구현으로 포함하지 않음.
// 원본 인코딩: utf-8-sig. UTF-8로 표시하고 BOM·줄바꿈을 정규화했습니다. 원문 주석/로그를 보존합니다.
// 본인 작성·수정행만 발췌했습니다. 함수/클래스 전체가 아니며 조각 사이의 선언·제어 흐름을 재구성하지 않았습니다.

// [원문 1-1] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
// Fill out your copyright notice in the Description page of Project Settings.

// [원문 10-10] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
#include "Blueprint/UserWidget.h"

// [원문 12-12] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
#include "UObject/NoExportTypes.h"

// [원문 32-34] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAbilityPointsChanged, float, NewPoints);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUpgradeModeToggled, bool, bActive);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAbilityUnlockTokensChanged, float, NewTokens);

// [원문 37-37] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.


// [원문 45-51] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.

	virtual void Jump() override;

	UPROPERTY(Replicated)
	FVector ReplicatedMovementInput;

	UFUNCTION(Server, Reliable)

// [원문 53-54] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.

	FVector GetReplicatedMovementInput() const { return ReplicatedMovementInput; }

// [원문 56-65] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
	// 레벨업 포인트 변환 듣기
	UPROPERTY(BlueprintAssignable, Category = "GAS|UI")
	FOnAbilityPointsChanged OnAbilityPointsChanged;

	UPROPERTY(BlueprintAssignable, Category = "GAS|UI")
	FOnUpgradeModeToggled OnUpgradeModeToggled;

	UPROPERTY(BlueprintAssignable, Category = "GAS|UI")
	FOnAbilityUnlockTokensChanged OnAbilityUnlockTokensChanged;


// [원문 101-101] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
	virtual void ApplyTeamCollision(int32 TeamIdx);

// [원문 115-115] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
	void OnAbilityPointsChangedNative(const FOnAttributeChangeData& Data);

// [원문 118-118] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
	virtual void OnRep_PlayerState() override;

// [원문 120-122] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.

	UPROPERTY()
	class UBaseMovementComponent* CachedBaseMovement;

// [원문 196-207] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAS")
	TArray<TSubclassOf<class UGameplayEffect>> InitialPassiveEffects;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AnimState")
	bool bIsCrouching = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AnimState")
	bool bIsAiming = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AnimState")
	bool bIsOutOfCombat = false;


// [원문 210-210] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.


// [원문 219-219] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.


// [원문 228-241] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
	UFUNCTION()
	void HandleWallClingStateChanged(bool bIsClinging);

	UFUNCTION()
	void HandleSlideStateChanged(bool bIsSliding);

	UPROPERTY(BlueprintReadWrite, Category = "UI")
	TObjectPtr<UUserWidget> SkillContainerWidget;

	UFUNCTION(BlueprintCallable, Category = "UI")
	void SetUpgradeMode(bool bActive);

	void OnAbilityUnlockTokensChangedNative(const FOnAttributeChangeData& Data);


// [원문 268-273] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Anims")
	TObjectPtr<UAnimMontage> AirDashMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Anims")
	TObjectPtr<UAnimMontage> GroundRollMontage;

// [원문 275-286] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Anims")
	TObjectPtr<UAnimMontage> DoubleJumpMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Anims")
	TObjectPtr<UAnimMontage> WallClingMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Anims")
	TObjectPtr<UAnimMontage> VaultMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Anims")
	TObjectPtr<UAnimMontage> SlideMontage;


// [원문 353-372] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.

	UFUNCTION(BlueprintCallable, Category = "Anim")
	float GetMontageEventTimeByTag(UAnimMontage* Montage, FGameplayTag EventTag);

	protected:
		UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "UI|HealthBar")
		TObjectPtr<class UWidgetComponent> OverheadHealthBarComp;

		void SetupOverheadHealthBarUI();
		bool IsEnemyToLocalPlayer() const;

		FTimerHandle OcclusionCheckTimerHandle;
		void CheckHealthBarOcclusion();

		UPROPERTY(EditDefaultsOnly, Category = "UI|HealthBar")
		float MaxHealthBarDrawDistance = 1500.0f;

		float LastKnownMaxHealth = -1.0f;

		FTimerHandle HealthBarRetryTimerHandle;

