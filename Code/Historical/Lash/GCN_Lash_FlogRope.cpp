// 개인 구현 및 수정 범위 전시용. 독립 빌드 대상이 아닙니다.
// 분류: A — 본인 단독 구현
// 원본: Source/TeamProject_DL/Lash/GCN_Lash_FlogRope.cpp
// 기준 커밋: 08a7c27856b77d63c8fd538bcd4ae071395be2bc
// 기여 범위: 본인이 신규 작성한 파일. 현재 삭제되어 최신 코드와 구분한 과거 구현으로 보존.
// 원본 인코딩: cp949. UTF-8로 표시하고 BOM·줄바꿈을 정규화했습니다. 원문 주석/로그를 보존합니다.
// 원문 전체입니다. 팀원 지원 수정이 있는 B 파일은 위 기여 범위와 SourceManifest에 명시합니다.

#include "GCN_Lash_FlogRope.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Common/SkillDamageLibrary.h" 

UGCN_Lash_FlogRope::UGCN_Lash_FlogRope()
{
	GameplayCueTag = FGameplayTag::RequestGameplayTag(FName("GameplayCue.Lash.FlogRope"));
}

bool UGCN_Lash_FlogRope::OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const
{
	AActor* InstigatorActor = Parameters.Instigator.Get();

	if (!FlogRopeNiagara || !InstigatorActor)
	{
		return false;
	}

	// 시작점
	FVector StartLocation = InstigatorActor->GetActorLocation();

	// Context에서 타겟 액터 배열 추출
	TArray<AActor*> TargetActors = USkillDamageLibrary::GetActorsFromEffectContext(Parameters.EffectContext);

	// 루프 돌며 나이아가라 스폰 및 파라미터 세팅
	for (AActor* Target : TargetActors)
	{
		if (!Target) continue;

		FVector EndLocation = Target->GetActorLocation() + FVector(0.f, 0.f, 50.f); // 가슴 높이 보정

		// 나이아가라 스폰
		UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			GetWorld(),
			FlogRopeNiagara,
			StartLocation
		);

		if (NiagaraComp)
		{
			// 시작점과 끝점 파라미터 주입
			NiagaraComp->SetVariableVec3(BeamStartParamName, StartLocation);
			NiagaraComp->SetVariableVec3(BeamEndParamName, EndLocation);
		}
	}

	return true;
}