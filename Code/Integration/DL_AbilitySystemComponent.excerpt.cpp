// 개인 구현 및 수정 범위 전시용. 독립 빌드 대상이 아닙니다.
// 분류: D — 팀원 시스템에 본인 기능 추가
// 원본: Source/TeamProject_DL/Private/DL_AbilitySystemComponent.cpp
// 기준 커밋: db9a1720265f38f776d8c1ff003933a356ebeef3
// 기여 범위: horimi_의 ASC에 GameplayTag로 첫 활성 효과의 남은 시간·지속시간을 조회하는 GetCooldownTimeByTag 추가.
// 원본 인코딩: utf-8-sig. UTF-8로 표시하고 BOM·줄바꿈을 정규화했습니다. 원문 주석/로그를 보존합니다.
// 본인 작성·수정행만 발췌했습니다. 함수/클래스 전체가 아니며 조각 사이의 선언·제어 흐름을 재구성하지 않았습니다.

// [원문 85-110] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.

void UDL_AbilitySystemComponent::GetCooldownTimeByTag(FGameplayTag CooldownTag, float& TimeRemaining, float& Duration)
{
    TimeRemaining = 0.f;
    Duration = 0.f;

    if (!CooldownTag.IsValid())
    {
        return;
    }

    // 캐릭터에게 걸려있는 "모든" 활성화된 이펙트(부적) 핸들을 다 가져옵니다.
    FGameplayEffectQuery Query = FGameplayEffectQuery::MakeQuery_MatchAllOwningTags(FGameplayTagContainer(CooldownTag));
    TArray<FActiveGameplayEffectHandle> ActiveEffects = GetActiveEffects(Query);

    if (ActiveEffects.Num() > 0)
    {
        // 이 태그를 가진 이펙트가 있다면, 첫 번째 이펙트의 시간 정보를 긁어옵니다!
        const FActiveGameplayEffect* ActiveEffect = GetActiveGameplayEffect(ActiveEffects[0]);
        if (ActiveEffect)
        {
            Duration = ActiveEffect->GetDuration();
            TimeRemaining = ActiveEffect->GetTimeRemaining(GetWorld()->GetTimeSeconds());
        }
    }
}

