// 개인 구현 및 수정 범위 전시용. 독립 빌드 대상이 아닙니다.
// 분류: B — 본인 구현 + 팀원 비기능 수정
// 원본: Source/TeamProject_DL/UI/InventoryHUDWidget.cpp
// 기준 커밋: db9a1720265f38f776d8c1ff003933a356ebeef3
// 기여 범위: 슬롯 캐싱·정렬·Pawn/인벤토리 바인딩은 본인. horimi_ 5005b20a4와 dalbi090211 70a21dc1b는 PlayerState.h include만 추가. 중복 include도 원문 그대로 보존.
// 원본 인코딩: utf-8-sig. UTF-8로 표시하고 BOM·줄바꿈을 정규화했습니다. 원문 주석/로그를 보존합니다.
// 원문 전체입니다. 팀원 지원 수정이 있는 B 파일은 위 기여 범위와 SourceManifest에 명시합니다.

#include "InventoryHUDWidget.h"
#include "InventorySlotWidget.h"
#include "DLInventoryComponent.h"
#include "DLItemSubsystem.h"
#include "GameCore/Character/PlayerCharacter.h"
#include "Components/UniformGridPanel.h"
#include "Components/UniformGridSlot.h"
#include "GameFramework/PlayerState.h"
#include "Engine/GameInstance.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/PlayerController.h" 

// [최적화 2] 정렬 시 O(N log N) 과정에서의 빈번한 데이터 조회(Find)를 막기 위한 임시 래퍼 구조체
struct FItemSortData
{
    FItemInstance Instance;
    const FItemDefinitionRow* Def;
};

void UInventoryHUDWidget::NativeConstruct()
{
    Super::NativeConstruct();

    UE_LOG(LogTemp, Warning, TEXT("InventoryHUD 켜짐!"));
    if (UGameInstance* GI = GetGameInstance())
    {
        ItemSubsystem = GI->GetSubsystem<UDLItemSubsystem>();
        if (!ItemSubsystem) UE_LOG(LogTemp, Error, TEXT("[HUD] ItemSubsystem을 찾을 수 없습니다!"));
    }

    // [최적화 1] 12개의 슬롯을 최초 1회만 생성하여 그리드에 배치 및 캐싱
    CachedSlots.Empty();
    if (InventoryGrid && SlotWidgetClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("슬롯 생성 시작!"));
        InventoryGrid->ClearChildren();
        const int32 TotalSlots = 12;

        for (int32 Index = 0; Index < TotalSlots; ++Index)
        {
            if (UInventorySlotWidget* SlotWidget = CreateWidget<UInventorySlotWidget>(this, SlotWidgetClass))
            {
                CachedSlots.Add(SlotWidget);
                if (UUniformGridSlot* GridSlot = InventoryGrid->AddChildToUniformGrid(SlotWidget))
                {
                    // 2행 6열 (열 우선) 배치 고정
                    GridSlot->SetRow(Index % 2);
                    GridSlot->SetColumn(Index / 2);
                }
            }
        }
        UE_LOG(LogTemp, Warning, TEXT("[HUD] 12개 슬롯 생성 및 캐싱 완료"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("[HUD] InventoryGrid 또는 SlotWidgetClass가 비어있어 슬롯 생성 실패!"));
    }

    // [최적화 3] PlayerController를 통해 Pawn 변경 이벤트 구독 (리스폰/빙의 대비)
    if (APlayerController* PC = GetOwningPlayer())
    {
        PC->OnPossessedPawnChanged.AddDynamic(this, &UInventoryHUDWidget::HandlePawnChanged);

        // UI가 생성될 시점에 이미 빙의된 폰이 있다면 즉시 바인딩 처리
        HandlePawnChanged(nullptr, PC->GetPawn());
    }
}

void UInventoryHUDWidget::NativeDestruct()
{
    UnbindInventoryEvents();

    // 폰 변경 이벤트 구독 해제
    if (APlayerController* PC = GetOwningPlayer())
    {
        PC->OnPossessedPawnChanged.RemoveDynamic(this, &UInventoryHUDWidget::HandlePawnChanged);
    }

    Super::NativeDestruct();
}

void UInventoryHUDWidget::HandlePawnChanged(APawn* OldPawn, APawn* NewPawn)
{
    UE_LOG(LogTemp, Warning, TEXT("[HUD] HandlePawnChanged 호출됨 (Old: %s, New: %s)"),
        OldPawn ? *OldPawn->GetName() : TEXT("None"),
        NewPawn ? *NewPawn->GetName() : TEXT("None"));

    UnbindInventoryEvents();

    // 폰이 바뀌었으니 타이머를 돌려서 PlayerState가 도착할 때까지 찾기 시작
    TryBindInventory();
}

void UInventoryHUDWidget::TryBindInventory()
{
    UDLInventoryComponent* FoundInventory = nullptr;

    // 1. PlayerController의 PlayerState에서 우선 검색 
    if (APlayerController* PC = GetOwningPlayer())
    {
        if (APlayerState* PS = PC->PlayerState)
        {
            FoundInventory = PS->FindComponentByClass<UDLInventoryComponent>();
        }
    }

    // 2. 못 찾았다면 Pawn의 PlayerState에서 검색
    if (!FoundInventory)
    {
        if (APawn* Pawn = GetOwningPlayerPawn())
        {
            if (APlayerState* PS = Pawn->GetPlayerState())
            {
                FoundInventory = PS->FindComponentByClass<UDLInventoryComponent>();
            }
        }
    }

    if (FoundInventory)
    {
        UE_LOG(LogTemp, Warning, TEXT("[HUD] 인벤토리 컴포넌트를 성공적으로 찾았습니다!"));
        BindInventoryEvents(FoundInventory);

        // 찾았으니 UI 한 번 새로고침
        RefreshInventoryUI();
    }
    else
    {
        // 네트워크 지연으로 인해 아직 PlayerState가 도착하지 않았으면 0.2초 뒤 다시 시도
        UE_LOG(LogTemp, Warning, TEXT("[HUD] 아직 InventoryComponent가 클라이언트에 복제되지 않았습니다. 0.2초 후 재시도합니다..."));
        GetWorld()->GetTimerManager().SetTimer(RetryTimerHandle, this, &UInventoryHUDWidget::TryBindInventory, 0.2f, false);
    }
}

void UInventoryHUDWidget::RefreshInventoryUI()
{
    UE_LOG(LogTemp, Warning, TEXT("[HUD] RefreshInventoryUI 호출됨!"));

    // CachedSlots가 비어있으면 초기화 실패 상태이므로 조기 종료
    if (!InventoryComponent)
    {
        UE_LOG(LogTemp, Error, TEXT("[HUD] Refresh 실패: InventoryComponent가 Null입니다."));
        return;
    }
    if (!ItemSubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("[HUD] Refresh 실패: ItemSubsystem이 Null입니다."));
        return;
    }
    if (CachedSlots.IsEmpty())
    {
        UE_LOG(LogTemp, Error, TEXT("[HUD] Refresh 실패: CachedSlots가 비어있습니다."));
        return;
    }

    FInventoryState State = InventoryComponent->GetInventoryStateCopy();
    UE_LOG(LogTemp, Warning, TEXT("[HUD] 현재 소유한 아이템 개수: %d개"), State.OwnedItems.Num());

    // [최적화 2] 정렬 전, FindItemDefinition 조회를 단 1회씩만 수행하여 데이터 구조체로 묶음
    TArray<FItemSortData> SortDataArray;
    SortDataArray.Reserve(State.OwnedItems.Num());

    for (const FItemInstance& Instance : State.OwnedItems)
    {
        if (const FItemDefinitionRow* Def = ItemSubsystem->FindItemDefinition(Instance.ItemId))
        {
            SortDataArray.Add({ Instance, Def });
            UE_LOG(LogTemp, Log, TEXT("[HUD] 데이터 찾음: %s"), *Def->DisplayName.ToString());
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("[HUD] DataTable에서 ItemId(%s)를 찾을 수 없습니다!"), *Instance.ItemId.ToString());
        }
    }

    // 정렬 (티어 우선 -> 카테고리 -> 가격)
    SortDataArray.Sort([](const FItemSortData& A, const FItemSortData& B)
        {
            if (A.Def->Tier != B.Def->Tier)
                return A.Def->Tier < B.Def->Tier;

            if (A.Def->Category != B.Def->Category)
                return A.Def->Category < B.Def->Category;

            if (A.Def->Price != B.Def->Price)
                return A.Def->Price < B.Def->Price;

            return A.Def->ItemId.LexicalLess(B.Def->ItemId);
        });

    // [최적화 1] 파괴/생성 반복 없이, 캐싱된 위젯에 데이터만 덮어쓰기
    for (int32 Index = 0; Index < CachedSlots.Num(); ++Index)
    {
        UInventorySlotWidget* SlotWidget = CachedSlots[Index];
        if (!SlotWidget) continue;

        if (Index < SortDataArray.Num())
        {
            const FItemSortData& SortData = SortDataArray[Index];

            // 시각적 데이터 갱신
            SlotWidget->SetItemData(SortData.Instance, *SortData.Def);
        }
        else
        {
            // 빈 공간 갱신
            SlotWidget->SetEmpty();
        }
    }
}


void UInventoryHUDWidget::HandleItemPurchased(FName ItemId, FGuid InstanceId)
{
    UE_LOG(LogTemp, Warning, TEXT("[HUD] HandleItemPurchased 이벤트 발생! (ItemId: %s)"), *ItemId.ToString());
    RefreshInventoryUI();
}

void UInventoryHUDWidget::HandleItemSold(FName ItemId, FGuid InstanceId)
{
	UE_LOG(LogTemp, Warning, TEXT("[HUD] HandleItemSold 이벤트 발생! (ItemId: %s)"), *ItemId.ToString());
    RefreshInventoryUI();
}

void UInventoryHUDWidget::BindInventoryEvents(UDLInventoryComponent* NewInventory)
{
    InventoryComponent = NewInventory;
    if (InventoryComponent)
    {
        InventoryComponent->OnInventoryChanged.AddDynamic(this, &UInventoryHUDWidget::RefreshInventoryUI);
        InventoryComponent->OnItemPurchased.AddDynamic(this, &UInventoryHUDWidget::HandleItemPurchased);
        InventoryComponent->OnItemSold.AddDynamic(this, &UInventoryHUDWidget::HandleItemSold);

        UE_LOG(LogTemp, Warning, TEXT("[HUD] InventoryComponent 델리게이트 바인딩 성공!"));
    }
}

void UInventoryHUDWidget::UnbindInventoryEvents()
{
    if (InventoryComponent)
    {
        InventoryComponent->OnInventoryChanged.RemoveDynamic(this, &UInventoryHUDWidget::RefreshInventoryUI);
        InventoryComponent->OnItemPurchased.RemoveDynamic(this, &UInventoryHUDWidget::HandleItemPurchased);
        InventoryComponent->OnItemSold.RemoveDynamic(this, &UInventoryHUDWidget::HandleItemSold);
        InventoryComponent = nullptr;

        UE_LOG(LogTemp, Warning, TEXT("[HUD] InventoryComponent 델리게이트 언바인딩 됨"));
    }
}