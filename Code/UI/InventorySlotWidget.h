// 개인 구현 및 수정 범위 전시용. 독립 빌드 대상이 아닙니다.
// 분류: A — 본인 단독 구현
// 원본: Source/TeamProject_DL/UI/InventorySlotWidget.h
// 기준 커밋: db9a1720265f38f776d8c1ff003933a356ebeef3
// 기여 범위: 본인이 신규 작성하고 후속 수정한 파일. 일반 커밋·병합 combined diff·현재 blame에서 팀원의 기능 변경을 확인하지 못함.
// 원본 인코딩: utf-8-sig. UTF-8로 표시하고 BOM·줄바꿈을 정규화했습니다. 원문 주석/로그를 보존합니다.
// 원문 전체입니다. 팀원 지원 수정이 있는 B 파일은 위 기여 범위와 SourceManifest에 명시합니다.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ItemTypes.h"
#include "InventorySlotWidget.generated.h"

class UImage;

UCLASS()
class TEAMPROJECT_DL_API UInventorySlotWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    void SetItemData(const FItemInstance& Instance, const FItemDefinitionRow& Def);
    void SetEmpty();

protected:
    //아이템 아이콘만 직접 제어
    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UImage> ItemIcon;

    // 블루프린트로 쿨타임 시작을 알리는 이벤트 (종료 시간, 전체 시간 전달)
    UFUNCTION(BlueprintImplementableEvent, Category = "Inventory|Cooldown")
    void BP_OnCooldownStarted(float EndTime, float Duration);

    // 블루프린트로 쿨타임 종료/아이템 비움을 알리는 이벤트
    UFUNCTION(BlueprintImplementableEvent, Category = "Inventory|Cooldown")
    void BP_OnCooldownStopped();
};