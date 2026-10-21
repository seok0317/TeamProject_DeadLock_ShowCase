// 개인 구현 및 수정 범위 전시용. 독립 빌드 대상이 아닙니다.
// 분류: D — 팀원 시스템에 본인 기능 추가
// 원본: Source/TeamProject_DL/Private/GA_SkillBase.cpp
// 기준 커밋: 985ace845d159f0baabf04c56036bbe40347e402
// 기여 범위: horimi_의 스킬 베이스에 무타깃 모드 서버 대기 우회/빈 TargetData 전송을 과거 추가. 최신 14fee7c08에서 팀원이 다른 구조로 대체; 현재 프레임워크를 본인 것으로 공개하지 않고 당시 본인 추가문만 별도 발췌.
// 원본 인코딩: utf-8-sig. UTF-8로 표시하고 BOM·줄바꿈을 정규화했습니다. 원문 주석/로그를 보존합니다.
// 본인 작성·수정행만 발췌했습니다. 함수/클래스 전체가 아니며 조각 사이의 선언·제어 흐름을 재구성하지 않았습니다.

// [원문 76-83] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
    // 최우석(수정) ESkillTargetingMode::None일때 서버가 밑에 if (bIsAuthority && !bIsLocallyControlled)에서 WaitTargetDataFromClient();으로 데이터를 받지 못해 
    // 일단 None일때 서버와 클라 동시에 스킬 시전하게 추가했습니다 
    if (TargetingMode == ESkillTargetingMode::None)
    {
        StartCast();
        return;
    }



// ===== 과거 후속 수정 스냅샷 =====

// 개인 구현 및 수정 범위 전시용. 독립 빌드 대상이 아닙니다.
// 분류: D — 팀원 시스템에 본인 기능 추가
// 원본: Source/TeamProject_DL/Private/GA_SkillBase.cpp
// 기준 커밋: 249fc259fce810baaf7a9101fa360da732b721e4
// 기여 범위: horimi_의 스킬 베이스에 무타깃 모드 서버 대기 우회/빈 TargetData 전송을 과거 추가. 최신 14fee7c08에서 팀원이 다른 구조로 대체; 현재 프레임워크를 본인 것으로 공개하지 않고 당시 본인 추가문만 별도 발췌.
// 원본 인코딩: utf-8-sig. UTF-8로 표시하고 BOM·줄바꿈을 정규화했습니다. 원문 주석/로그를 보존합니다.
// 본인 작성·수정행만 발췌했습니다. 함수/클래스 전체가 아니며 조각 사이의 선언·제어 흐름을 재구성하지 않았습니다.

// [원문 90-94] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
        if (!bIsAuthority)
        {
            FGameplayAbilityTargetDataHandle EmptyDataHandle; // 빈 무전기
            SendTargetDataToServer(EmptyDataHandle);          // 서버한테 쏴버림!
        }

