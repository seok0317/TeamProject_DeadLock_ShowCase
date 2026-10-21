// 개인 구현 및 수정 범위 전시용. 독립 빌드 대상이 아닙니다.
// 분류: C — 팀원과 기능 공동 관여
// 원본: Source/TeamProject_DL/GameCore/Character/PlayerCharacter.cpp
// 기준 커밋: db9a1720265f38f776d8c1ff003933a356ebeef3
// 기여 범위: 캐릭터 뼈대는 dalbi090211, 전투/집라인은 Youngwoo, 스킬 입력/상점 등은 horimi_. 본인은 카메라 기준 이동 입력·대시/슬라이드·UI 모드·이동 상태 연동 범위만 발췌.
// 원본 인코딩: utf-8-sig. UTF-8로 표시하고 BOM·줄바꿈을 정규화했습니다. 원문 주석/로그를 보존합니다.
// 본인 작성·수정행만 발췌했습니다. 함수/클래스 전체가 아니며 조각 사이의 선언·제어 흐름을 재구성하지 않았습니다.

// [원문 1-1] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
#include "PlayerCharacter.h"

// [원문 6-6] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
#include "GameFramework/CharacterMovementComponent.h"

// [원문 8-10] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
#include "MovementComponent/BaseMovementComponent.h"
#include "EnhancedInputSubsystems.h" 
#include "EnhancedInputComponent.h" 

// [원문 12-12] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
#include "AbilitySystemComponent.h"

// [원문 15-15] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
#include "GameplayTagContainer.h"

// [원문 31-32] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
APlayerCharacter::APlayerCharacter(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer) 

// [원문 34-40] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);


// [원문 43-48] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
	CameraBoom->TargetArmLength = 150.0f;
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->CameraLagSpeed = 15.0f;
	CameraBoom->SocketOffset = FVector(0.0f, 70.0f, 60.0f);
	CameraBoom->TargetOffset = FVector(0.0f, 0.0f, 0.0f);

// [원문 52-52] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
	FollowCamera->bUsePawnControlRotation = false;

// [원문 244-245] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
}


// [원문 393-423] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.

void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}
	}

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveAction) EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &APlayerCharacter::InputMove);
		if (LookAction) EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &APlayerCharacter::InputLook);
		if (DashAction) EnhancedInputComponent->BindAction(DashAction, ETriggerEvent::Started, this, &APlayerCharacter::InputDash);
		if (JumpAction)
		{
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &APlayerCharacter::InputJumpStart);
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &APlayerCharacter::InputJumpEnd);
		}
		if (CrouchSlideAction)
		{
			EnhancedInputComponent->BindAction(CrouchSlideAction, ETriggerEvent::Started, this, &APlayerCharacter::InputCrouchSlideStart);
			EnhancedInputComponent->BindAction(CrouchSlideAction, ETriggerEvent::Completed, this, &APlayerCharacter::InputCrouchSlideEnd);
		}

// [원문 442-443] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
		if (BasicAttackAction) EnhancedInputComponent->BindAction
		(

// [원문 461-461] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.


// [원문 484-488] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
		// Tab을 누를 때 (Started)
		EnhancedInputComponent->BindAction(UpgradeMenuAction, ETriggerEvent::Started, this, &APlayerCharacter::InputTabPressed);
		// Tab을 뗄 때 (Completed)
		EnhancedInputComponent->BindAction(UpgradeMenuAction, ETriggerEvent::Completed, this, &APlayerCharacter::InputTabReleased);


// [원문 510-514] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
	}
}

void APlayerCharacter::InputMove(const FInputActionValue& Value)
{

// [원문 516-523] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
	UAbilitySystemComponent* MyASC = GetAbilitySystemComponent();
	if (MyASC)
	{
		static const FGameplayTag CubedTag = FGameplayTag::RequestGameplayTag(FName("State.Status.Cubed"), false);
		static const FGameplayTag ParryingTag = FGameplayTag::RequestGameplayTag(FName("State.Combat.Parrying"), false);
		static const FGameplayTag RollingTag = FGameplayTag::RequestGameplayTag(FName("State.Movement.Rolling"), false);
		static const FGameplayTag InplaceSkillTag = FGameplayTag::RequestGameplayTag(FName("State.Combat.InplaceSkill"), false);


// [원문 536-541] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
		if (RollingTag.IsValid() && MyASC->HasMatchingGameplayTag(RollingTag))
		{
			UE_LOG(LogTemp, Warning, TEXT("Rolling 중 이동 차단"));
			ConsumeMovementInputVector();
			return; 
		}

// [원문 543-543] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
		if (InplaceSkillTag.IsValid() && MyASC->HasMatchingGameplayTag(InplaceSkillTag))

// [원문 546-546] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
			UE_LOG(LogTemp, Warning, TEXT("Skill 중 이동 차단"));

// [원문 549-575] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
	}

	FVector2D MovementVector = Value.Get<FVector2D>();
	if (Controller != nullptr)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void APlayerCharacter::InputLook(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();
	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void APlayerCharacter::InputJumpStart(const FInputActionValue& Value)
{

// [원문 586-588] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
	Jump();
}


// [원문 596-596] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
	static const FGameplayTag ParryingTag = FGameplayTag::RequestGameplayTag(FName("State.Combat.Parrying"), false);

// [원문 609-610] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
	static const FGameplayTag ZiplineStateTag = FGameplayTag::RequestGameplayTag(TEXT("State.Movement.Zipline"), false);
	static const FGameplayTag ZiplineAbilityTag = FGameplayTag::RequestGameplayTag(TEXT("Ability.Movement.Zipline"), false);

// [원문 631-633] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
void APlayerCharacter::InputJumpEnd(const FInputActionValue& Value)
{
	StopJumping();

// [원문 635-643] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.


void APlayerCharacter::InputDash(const FInputActionValue& Value)
{
	UAbilitySystemComponent* MyASC = GetAbilitySystemComponent();
	UBaseMovementComponent* CMC = CachedBaseMovement;
	if (!MyASC || !CMC) return;

	static const FGameplayTag HeavyMeleeLungingTag = FGameplayTag::RequestGameplayTag(FName("State.Combat.HeavyMeleeLunging"), false);

// [원문 649-649] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
	static const FGameplayTag ParryingTag = FGameplayTag::RequestGameplayTag(FName("State.Combat.Parrying"), false);

// [원문 655-681] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
	// 제자리 무시
	if (CMC->GetLastInputVector().IsNearlyZero()) return;

	// 공용 입력 벡터 구하기
	FVector InputDir = CMC->GetLastInputVector().GetSafeNormal();

	// 공용 입력 동기화 RPC 호출
	ServerSetMovementInput(InputDir);

	if (CMC->IsFalling())
	{
		// 공중인 경우
		static const FGameplayTag AirDashTag = FGameplayTag::RequestGameplayTag(FName("Ability.Movement.AirDash"), false);
		if (AirDashTag.IsValid())
		{
			MyASC->TryActivateAbilitiesByTag(FGameplayTagContainer(AirDashTag));
		}
	}
	else
	{
		// 지면인 경우
		static const FGameplayTag GroundRollTag = FGameplayTag::RequestGameplayTag(FName("Ability.Movement.GroundRoll"), false);
		if (GroundRollTag.IsValid())
		{
			MyASC->TryActivateAbilitiesByTag(FGameplayTagContainer(GroundRollTag));
		}
	}

// [원문 767-774] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
void APlayerCharacter::InputCrouchSlideStart(const FInputActionValue& Value)
{
	UAbilitySystemComponent* MyASC = GetAbilitySystemComponent();
	UBaseMovementComponent* CMC = CachedBaseMovement;

	if (!MyASC || !CMC) return;

	static const FGameplayTag HeavyMeleeLungingTag = FGameplayTag::RequestGameplayTag(FName("State.Combat.HeavyMeleeLunging"), false);

// [원문 780-780] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
	static const FGameplayTag ParryingTag = FGameplayTag::RequestGameplayTag(FName("State.Combat.Parrying"), false);

// [원문 786-810] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.
	// 현재 속도나 경사로 조건을 만족하면 슬라이딩 발동
	if (CMC->CanSlide())
	{
		UE_LOG(LogTemp, Warning, TEXT("슬라이드 시작"));
		static const FGameplayTag SlideTag = FGameplayTag::RequestGameplayTag(FName("Ability.Movement.Slide"), false);
		if (SlideTag.IsValid())
		{
			MyASC->TryActivateAbilitiesByTag(FGameplayTagContainer(SlideTag));
		}
	}
}

void APlayerCharacter::InputCrouchSlideEnd(const FInputActionValue& Value)
{
	UAbilitySystemComponent* MyASC = GetAbilitySystemComponent();

	if (!MyASC) return;

	// 손을 떼면 슬라이딩 어빌리티 강제 종료 (Cancel)
	static const FGameplayTag SlideAbilityTag = FGameplayTag::RequestGameplayTag(FName("Ability.Movement.Slide"), false);
	if (SlideAbilityTag.IsValid())
	{
		FGameplayTagContainer TagsToCancel(SlideAbilityTag);
		MyASC->CancelAbilities(&TagsToCancel);
	}

// [원문 981-991] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.

void APlayerCharacter::InputTabPressed()
{
	SetUpgradeMode(true);
}

void APlayerCharacter::InputTabReleased()
{
	SetUpgradeMode(false);
}


// [원문 1294-1294] 본인 기여 구간. 앞뒤 문맥은 생략되어 있습니다.


