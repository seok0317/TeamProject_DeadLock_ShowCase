// 개인 구현 및 수정 범위 전시용. 독립 빌드 대상이 아닙니다.
// 분류: D — 팀원 시스템에 본인 기능 추가
// 원본: Source/TeamProject_DL/GameCore/Struct/ACharacterAttributeSet.cpp
// 기준 커밋: db9a1720265f38f776d8c1ff003933a356ebeef3
// 기여 범위: dalbi090211 등의 스탯 클래스에 GE 적용 후 스태미나 Base Value 상한 보정을 추가. 스탯 생성·피해·체력 계산 전체 제외. 로그 주석 처리는 보조 정리.
// 원본 인코딩: utf-8-sig. UTF-8로 표시하고 BOM·줄바꿈을 정규화했습니다. 원문 주석/로그를 보존합니다.
// 본인 작성·수정행만 발췌했습니다. 함수/클래스 전체가 아니며 조각 사이의 선언·제어 흐름을 재구성하지 않았습니다.

// [원문 167-180] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
	// GE로 인한 Stamina 증가가 MaxStamina보다 커지는	경우를 방지하기 위해, MaxStamina를 초과하는 경우 강제로 MaxStamina로 덮어씌움
	if (Data.EvaluatedData.Attribute == GetStaminaAttribute())
	{
		float MaxStamValue = GetMaxStamina();

		// GetStamina()가 아니라 '뿌리 값(Base Value)'을 직접 가져옵니다.
		float BaseStamValue = GetOwningAbilitySystemComponent()->GetNumericAttributeBase(GetStaminaAttribute());

		if (BaseStamValue > MaxStamValue)
		{
			// 겉모습뿐만 아니라 '뿌리 값' 자체를 최대치로 강제 덮어씌웁니다!
			GetOwningAbilitySystemComponent()->SetNumericAttributeBase(GetStaminaAttribute(), MaxStamValue);
		}
	}

