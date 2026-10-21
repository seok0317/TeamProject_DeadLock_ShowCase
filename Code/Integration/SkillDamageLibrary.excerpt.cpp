// 개인 구현 및 수정 범위 전시용. 독립 빌드 대상이 아닙니다.
// 분류: D — 팀원 시스템에 본인 기능 추가
// 원본: Source/TeamProject_DL/Common/SkillDamageLibrary.cpp
// 기준 커밋: db9a1720265f38f776d8c1ff003933a356ebeef3
// 기여 범위: dalbi090211/Youngwoo의 피해 파이프라인에 EffectContext 다중 액터 추출과 넥서스 아군 피해/무적 차단을 추가. 기본 피해 공식·GE 파이프라인 전체는 팀원.
// 원본 인코딩: utf-8-sig. UTF-8로 표시하고 BOM·줄바꿈을 정규화했습니다. 원문 주석/로그를 보존합니다.
// 본인 작성·수정행만 발췌했습니다. 함수/클래스 전체가 아니며 조각 사이의 선언·제어 흐름을 재구성하지 않았습니다.

// [원문 14-14] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
#include "GameCore/GameRule/Nexus.h"

// [원문 181-198] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
	// [팀킬(Friendly Fire) 및 무적 방지 - 넥서스 전용 예외 처리]
	if (TargetActor && TargetActor->IsA<ANexus>())
	{
		ANexus* TargetNexus = Cast<ANexus>(TargetActor);
		
		// 1. 넥서스가 무적(Phase 1 대기 상태 등)이라면 스킬 데미지 씹기
		if (TargetNexus && TargetNexus->bIsInvincible)
		{
			return;
		}

		// 2. 공격자가 같은 팀이라면 팀킬 데미지 씹기
		if (!SkillTeamUtils::IsHostile(SourceActor, TargetActor))
		{
			return; 
		}
	}


// [원문 264-273] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
	// [넥서스 무적 방어] 장판 스킬 등 ApplyDamage를 거치지 않는 데미지 방어
	if (TargetActor && TargetActor->IsA<ANexus>())
	{
		ANexus* TargetNexus = Cast<ANexus>(TargetActor);
		if (TargetNexus && TargetNexus->bIsInvincible)
		{
			return; // 무적 상태면 데미지 GE 적용을 완전히 차단
		}
	}
	

// [원문 283-303] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.

TArray<AActor*> USkillDamageLibrary::GetActorsFromEffectContext(FGameplayEffectContextHandle EffectContext)
{
	TArray<AActor*> ResultActors;

	if (!EffectContext.IsValid())
	{
		return ResultActors;
	}

	// AddActors()로 전달받은 액터 배열을 안전하게 추출합니다.
	for (const TWeakObjectPtr<AActor>& WeakActor : EffectContext.GetActors())
	{
		if (AActor* Actor = WeakActor.Get())
		{
			ResultActors.AddUnique(Actor);
		}
	}

	return ResultActors;
}

