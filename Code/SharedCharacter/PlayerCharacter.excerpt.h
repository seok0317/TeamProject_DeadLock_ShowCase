// 개인 구현 및 수정 범위 전시용. 독립 빌드 대상이 아닙니다.
// 분류: C — 팀원과 기능 공동 관여
// 원본: Source/TeamProject_DL/GameCore/Character/PlayerCharacter.h
// 기준 커밋: db9a1720265f38f776d8c1ff003933a356ebeef3
// 기여 범위: 공통 플레이어 클래스에서 본인의 카메라·이동 입력·슬라이드/UI 관련 선언만 발췌.
// 원본 인코딩: utf-8-sig. UTF-8로 표시하고 BOM·줄바꿈을 정규화했습니다. 원문 주석/로그를 보존합니다.
// 본인 작성·수정행만 발췌했습니다. 함수/클래스 전체가 아니며 조각 사이의 선언·제어 흐름을 재구성하지 않았습니다.

// [원문 7-7] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
#include "InputActionValue.h"

// [원문 10-11] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
class UInputMappingContext;
class UInputAction;

// [원문 29-29] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
	APlayerCharacter(const FObjectInitializer& ObjectInitializer);

// [원문 94-96] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
protected:

	virtual void BeginPlay() override;

// [원문 100-109] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	//UDeadLockMovementComponent* GetCustomMovementComponent() const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputMappingContext* DefaultMappingContext;


// [원문 142-156] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* MoveAction; // WASD 이동용

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* LookAction; // 마우스 회전용

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* DashAction; // Shift 대시용

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* CrouchSlideAction;


// [원문 172-174] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* UpgradeMenuAction;


// [원문 230-231] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
	void InputCrouchSlideStart(const FInputActionValue& Value);
	void InputCrouchSlideEnd(const FInputActionValue& Value);

// [원문 242-243] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
	void InputTabPressed();
	void InputTabReleased();

// [원문 269-269] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.


