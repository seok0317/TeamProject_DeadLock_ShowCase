// 개인 구현 및 수정 범위 전시용. 독립 빌드 대상이 아닙니다.
// 분류: A — 본인 단독 구현
// 원본: Source/TeamProject_DL/UI/InventoryHUDWidget.h
// 기준 커밋: db9a1720265f38f776d8c1ff003933a356ebeef3
// 기여 범위: 본인이 신규 작성하고 후속 수정한 파일. 일반 커밋·병합 combined diff·현재 blame에서 팀원의 기능 변경을 확인하지 못함.
// 원본 인코딩: utf-8-sig. UTF-8로 표시하고 BOM·줄바꿈을 정규화했습니다. 원문 주석/로그를 보존합니다.
// 원문 전체입니다. 팀원 지원 수정이 있는 B 파일은 위 기여 범위와 SourceManifest에 명시합니다.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ItemTypes.h"
#include "InventoryHUDWidget.generated.h"

class UUniformGridPanel;
class UInventorySlotWidget;
class UDLInventoryComponent;
class UDLItemSubsystem;

UCLASS()
class TEAMPROJECT_DL_API UInventoryHUDWidget : public UUserWidget
{
    GENERATED_BODY()

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    UPROPERTY(meta = (BindWidget))
    TObjectPtr<UUniformGridPanel> InventoryGrid;

    UPROPERTY(EditDefaultsOnly, Category = "Inventory|Widget")
    TSubclassOf<UInventorySlotWidget> SlotWidgetClass;

private:
    UPROPERTY(Transient)
    TObjectPtr<UDLInventoryComponent> InventoryComponent;

    UPROPERTY(Transient)
    TObjectPtr<UDLItemSubsystem> ItemSubsystem;

    // [최적화 1] 생성/파괴 비용 절감을 위한 슬롯 위젯 오브젝트 풀
    UPROPERTY(Transient)
    TArray<TObjectPtr<UInventorySlotWidget>> CachedSlots;

    UFUNCTION()
    void RefreshInventoryUI();

    UFUNCTION()
    void HandleItemPurchased(FName ItemId, FGuid InstanceId);

    UFUNCTION()
    void HandleItemSold(FName ItemId, FGuid InstanceId);

    // [최적화 2] 캐릭터 사망/리스폰(빙의) 시 델리게이트 안전망
    UFUNCTION()
    void HandlePawnChanged(APawn* OldPawn, APawn* NewPawn);

    UFUNCTION()
    void TryBindInventory();

    FTimerHandle RetryTimerHandle;

    void BindInventoryEvents(UDLInventoryComponent* NewInventory);
    void UnbindInventoryEvents();
};