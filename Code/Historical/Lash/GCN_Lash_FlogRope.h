// 개인 구현 및 수정 범위 전시용. 독립 빌드 대상이 아닙니다.
// 분류: A — 본인 단독 구현
// 원본: Source/TeamProject_DL/Lash/GCN_Lash_FlogRope.h
// 기준 커밋: 08a7c27856b77d63c8fd538bcd4ae071395be2bc
// 기여 범위: 본인이 신규 작성한 파일. 현재 삭제되어 최신 코드와 구분한 과거 구현으로 보존.
// 원본 인코딩: utf-8-sig. UTF-8로 표시하고 BOM·줄바꿈을 정규화했습니다. 원문 주석/로그를 보존합니다.
// 원문 전체입니다. 팀원 지원 수정이 있는 B 파일은 위 기여 범위와 SourceManifest에 명시합니다.

#pragma once

#include "CoreMinimal.h"
#include "GameplayCueNotify_Burst.h"
#include "GCN_Lash_FlogRope.generated.h"

class UNiagaraSystem;

UCLASS()
class TEAMPROJECT_DL_API UGCN_Lash_FlogRope : public UGameplayCueNotify_Burst
{
	GENERATED_BODY()

public:
	UGCN_Lash_FlogRope();

	virtual bool OnExecute_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters) const override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Visual|Niagara")
	TObjectPtr<UNiagaraSystem> FlogRopeNiagara;

	UPROPERTY(EditDefaultsOnly, Category = "Visual|Niagara")
	FName BeamStartParamName = FName("BeamStart");

	UPROPERTY(EditDefaultsOnly, Category = "Visual|Niagara")
	FName BeamEndParamName = FName("BeamEnd");
};