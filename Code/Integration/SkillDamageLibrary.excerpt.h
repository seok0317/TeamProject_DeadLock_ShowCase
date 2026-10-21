// 개인 구현 및 수정 범위 전시용. 독립 빌드 대상이 아닙니다.
// 분류: D — 팀원 시스템에 본인 기능 추가
// 원본: Source/TeamProject_DL/Common/SkillDamageLibrary.h
// 기준 커밋: db9a1720265f38f776d8c1ff003933a356ebeef3
// 기여 범위: 다중 대상 Cue 연결용 GetActorsFromEffectContext 선언 추가만 본인.
// 원본 인코딩: utf-8-sig. UTF-8로 표시하고 BOM·줄바꿈을 정규화했습니다. 원문 주석/로그를 보존합니다.
// 본인 작성·수정행만 발췌했습니다. 함수/클래스 전체가 아니며 조각 사이의 선언·제어 흐름을 재구성하지 않았습니다.

// [원문 35-37] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
	UFUNCTION(BlueprintCallable, Category = "GAS|SkillUtils", meta = (DisplayName = "Get Actors From Effect Context"))
	static TArray<AActor*> GetActorsFromEffectContext(FGameplayEffectContextHandle EffectContext);


// [원문 49-49] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.


