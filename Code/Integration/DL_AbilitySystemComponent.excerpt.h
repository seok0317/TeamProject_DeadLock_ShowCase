// 개인 구현 및 수정 범위 전시용. 독립 빌드 대상이 아닙니다.
// 분류: D — 팀원 시스템에 본인 기능 추가
// 원본: Source/TeamProject_DL/Public/DL_AbilitySystemComponent.h
// 기준 커밋: db9a1720265f38f776d8c1ff003933a356ebeef3
// 기여 범위: 쿨타임 조회 함수 선언만 기능 기여. 기존 강화 함수에 대한 한글 주석 복구는 기능 소유권 근거에서 제외.
// 원본 인코딩: utf-8-sig. UTF-8로 표시하고 BOM·줄바꿈을 정규화했습니다. 원문 주석/로그를 보존합니다.
// 본인 작성·수정행만 발췌했습니다. 함수/클래스 전체가 아니며 조각 사이의 선언·제어 흐름을 재구성하지 않았습니다.

// [원문 31-31] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
    // 더 올릴 수 있는지(최대 미만인지) 확인. UI 버튼/헛된 RPC 방지용.

// [원문 34-37] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.

    // 쿨타임을 가져오기 위해 추가 
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Ability|Cooldown")
    void GetCooldownTimeByTag(FGameplayTag CooldownTag, float& TimeRemaining, float& Duration);

