// 개인 구현 및 수정 범위 전시용. 독립 빌드 대상이 아닙니다.
// 분류: A — 본인 단독 구현
// 원본: Source/TeamProject_DL/MovementComponent/BaseMovementComponent.cpp
// 기준 커밋: db9a1720265f38f776d8c1ff003933a356ebeef3
// 기여 범위: 본인이 신규 작성하고 후속 수정한 파일. 일반 커밋·병합 combined diff·현재 blame에서 팀원의 기능 변경을 확인하지 못함.
// 원본 인코딩: utf-8-sig. UTF-8로 표시하고 BOM·줄바꿈을 정규화했습니다. 원문 주석/로그를 보존합니다.
// 원문 전체입니다. 팀원 지원 수정이 있는 B 파일은 위 기여 범위와 SourceManifest에 명시합니다.

#include "../MovementComponent/BaseMovementComponent.h"
#include "GameFramework/Character.h"
#include "../GameCore/Character/BaseCharacter.h"
#include "AbilitySystemComponent.h"
#include "GameplayTagContainer.h"
#include "Kismet/KismetSystemLibrary.h"

void UBaseMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	CachedBaseChar = Cast<ABaseCharacter>(GetOwner());
}

void UBaseMovementComponent::PrepareAirDash(FVector DashDirection)
{
	DefaultGravityScale = GravityScale;
	CachedPreDashVelocity = FVector(Velocity.X / 1.5f, Velocity.Y / 1.5f, 0.0f);
	Velocity.X *= 0.33f;
	Velocity.Y *= 0.33f;
	Velocity.Z = 0.0f;
	ReleaseWallCling();
}

void UBaseMovementComponent::ResetGravity() { GravityScale = DefaultGravityScale; }

void UBaseMovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (MovementMode == MOVE_Falling) 
	{ 
		UpdateWallCling(DeltaTime);
	}

}

void UBaseMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);
	if (MovementMode != MOVE_Falling) { ReleaseWallCling(); }
}

void UBaseMovementComponent::UpdateWallCling(float DeltaTime)
{
	if (!CachedBaseChar || CachedBaseChar->GetLocalRole() == ROLE_SimulatedProxy) return;

	if (bDisableWallCling)
	{
		ReleaseWallCling(); 
		return; 
	}

	FVector VaultTargetLoc;
	float VaultWallHeight;
	if (CheckVaulting(VaultTargetLoc, VaultWallHeight)) // 타는 벽이 아니라 넘는 벽이면 return
	{
		ReleaseWallCling();
		return;
	}

	FVector StartLoc = CachedBaseChar->GetActorLocation();
	FVector ForwardTrace = StartLoc + (CachedBaseChar->GetActorForwardVector() * WallClingTraceDistance);
	FVector RightTrace = StartLoc + (CachedBaseChar->GetActorRightVector() * WallClingTraceDistance);
	FVector LeftTrace = StartLoc + (-CachedBaseChar->GetActorRightVector() * WallClingTraceDistance);
	FVector BackwardTrace = StartLoc + (-CachedBaseChar->GetActorForwardVector() * WallClingTraceDistance);


	FVector TraceEnds[4] = { ForwardTrace, RightTrace, LeftTrace, BackwardTrace };

	bool bFoundWall = false;
	FVector FoundNormal = FVector::ZeroVector;

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(CachedBaseChar);

	for (const FVector& EndLoc : TraceEnds)
	{
		FHitResult HitResult;
		bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, StartLoc, EndLoc, ECC_Visibility, QueryParams);

		if (bHit && FMath::Abs(HitResult.Normal.Z) < 0.2f)
		{
			bFoundWall = true;
			FoundNormal = HitResult.Normal;
			break;
		}
	}

	if (bFoundWall)
	{
		if (!bIsWallClinging)
		{
			bIsWallClinging = true;
			CachedWallNormal = FoundNormal;

			OnWallClingStateChanged.Broadcast(true);
		}
		else 
		{ 
			CachedWallNormal = FoundNormal; 

			// 서버 동기화 방어 코드
			if (CachedBaseChar->GetLocalRole() == ROLE_Authority)
			{
				OnWallClingStateChanged.Broadcast(true);
			}
		}
	}
	else { ReleaseWallCling(); }
}

void UBaseMovementComponent::ReleaseWallCling()
{
	if (bIsWallClinging)
	{
		bIsWallClinging = false;
		OnWallClingStateChanged.Broadcast(false);
	}
}



bool UBaseMovementComponent::FSavedMove_BaseMovement::CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const
{
	// 연속된 프레임에서 슬라이딩 상태가 같다면 패킷을 합쳐서 대역폭 절약
	FSavedMove_BaseMovement* NewBaseMove = static_cast<FSavedMove_BaseMovement*>(NewMove.Get());
	if (bSavedIsSliding != NewBaseMove->bSavedIsSliding) { return false; }
	if (bSavedIsWallClinging != NewBaseMove->bSavedIsWallClinging) { return false; }
	return Super::CanCombineWith(NewMove, InCharacter, MaxDelta);
}

void UBaseMovementComponent::FSavedMove_BaseMovement::Clear()
{
	Super::Clear();
	bSavedIsSliding = 0;
	bSavedIsWallClinging = 0;
}

uint8 UBaseMovementComponent::FSavedMove_BaseMovement::GetCompressedFlags() const
{
	uint8 Result = Super::GetCompressedFlags();
	// 슬라이딩 상태를 언리얼 엔진의 Custom Flag 0번에 압축해서 넣음
	if (bSavedIsSliding) { Result |= FLAG_Custom_0; }
	// 벽 매달리기 상태는 Custom Flag 1번에 압축
	if (bSavedIsWallClinging) { Result |= FLAG_Custom_1; }
	return Result;
}

void UBaseMovementComponent::FSavedMove_BaseMovement::SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData)
{
	Super::SetMoveFor(C, InDeltaTime, NewAccel, ClientData);
	UBaseMovementComponent* CharacterMovement = Cast<UBaseMovementComponent>(C->GetCharacterMovement());
	if (CharacterMovement)
	{
		// 클라이언트의 현재 상태 캡처
		bSavedIsSliding = CharacterMovement->bIsSliding;
		bSavedIsWallClinging = CharacterMovement->bIsWallClinging;
	}
}

void UBaseMovementComponent::FSavedMove_BaseMovement::PrepMoveFor(ACharacter* C)
{
	Super::PrepMoveFor(C);
	UBaseMovementComponent* CharacterMovement = Cast<UBaseMovementComponent>(C->GetCharacterMovement());
	if (CharacterMovement)
	{
		// 서버가 패킷을 받아 압축을 풀고 자신의 상태를 클라이언트와 동일하게 덮어씌움
		CharacterMovement->bIsSliding = (bSavedIsSliding != 0);
		CharacterMovement->bIsWallClinging = (bSavedIsWallClinging != 0);
	}
}

UBaseMovementComponent::FNetworkPredictionData_Client_BaseMovement::FNetworkPredictionData_Client_BaseMovement(const UCharacterMovementComponent& ClientMovement)
	: Super(ClientMovement) {
}

FSavedMovePtr UBaseMovementComponent::FNetworkPredictionData_Client_BaseMovement::AllocateNewMove()
{
	return FSavedMovePtr(new FSavedMove_BaseMovement());
}

FNetworkPredictionData_Client* UBaseMovementComponent::GetPredictionData_Client() const
{
	if (!ClientPredictionData)
	{
		UBaseMovementComponent* MutableThis = const_cast<UBaseMovementComponent*>(this);
		MutableThis->ClientPredictionData = new FNetworkPredictionData_Client_BaseMovement(*this);
		MutableThis->ClientPredictionData->MaxSmoothNetUpdateDist = 92.f;
		MutableThis->ClientPredictionData->NoSmoothNetUpdateDist = 140.f;
	}
	return ClientPredictionData;
}

void UBaseMovementComponent::UpdateCharacterStateBeforeMovement(float DeltaSeconds)
{
	if (bIsSliding) { bWantsToCrouch = true; }
	else { bWantsToCrouch = false; }

	Super::UpdateCharacterStateBeforeMovement(DeltaSeconds);
}

void UBaseMovementComponent::StartSlide()
{
	if (!CanSlide() || bIsSliding) return;
	bIsSliding = true;
	bWantsToCrouch = true;
}

void UBaseMovementComponent::StopSlide()
{
	if (!bIsSliding) return;
	bIsSliding = false;
	bWantsToCrouch = false;
}

bool UBaseMovementComponent::CanSlide() const
{
	if (!IsMovingOnGround()) return false;

	float CurrentSpeed = Velocity.Size2D();
	FVector FloorNormal = CurrentFloor.HitResult.Normal;
	bool bIsOnSlope = FloorNormal.Z < 0.99f;

	// 1. 평지 조건: 지정한 최소 속도(400) 이상으로 뛸 때만 가능
	bool bValidFlatSlide = (CurrentSpeed >= MinSlideSpeed);

	// 2. 경사로 조건 세밀화
	bool bValidSlopeSlide = false;
	if (bIsOnSlope)
	{
		// 경사로의 가장 가파른 내리막길 벡터 계산
		FVector GravityDir = FVector(0.0f, 0.0f, -1.0f);
		FVector SlopeDown = (GravityDir - FVector::DotProduct(GravityDir, FloorNormal) * FloorNormal).GetSafeNormal();

		// 내 캐릭터가 바라보는 방향
		FVector Forward = GetCharacterOwner()->GetActorForwardVector();

		// [조건 A] 캐릭터가 내리막길 방향을 바라보는가? (내적 0 이상 = 같은 방향)
		bool bFacingDownhill = FVector::DotProduct(Forward, SlopeDown) > 0.0f;

		// [조건 B] 정지 상태가 아닌가? (최소 150 이상의 속도는 붙어 있어야 함)
		float MinSlopeStartSpeed = 50.0f;

		if (bFacingDownhill && CurrentSpeed >= MinSlopeStartSpeed)
		{
			bValidSlopeSlide = true;
		}
	}

	// 평지 조건이거나 경사로 조건일 때만 슬라이딩 허용
	return bValidFlatSlide || bValidSlopeSlide;
}


void UBaseMovementComponent::UpdateCharacterStateAfterMovement(float DeltaSeconds)
{
	Super::UpdateCharacterStateAfterMovement(DeltaSeconds);
}

void UBaseMovementComponent::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);

	// FLAG_Custom_0 도장이 찍혀있다면 슬라이딩 상태를 true로 만듬
	bIsSliding = (Flags & FSavedMove_Character::FLAG_Custom_0) != 0;
	// FLAG_Custom_1 도장이 찍혀있다면 벽 매달리기 상태를 true로 만듬
	bIsWallClinging = (Flags & FSavedMove_Character::FLAG_Custom_1) != 0;
}

void UBaseMovementComponent::CalcVelocity(float DeltaTime, float Friction, bool bFluid, float BrakingDeceleration)
{
	// 슬라이딩이 아닐 때는 언리얼 엔진 기본 걷기/달리기 물리 사용
	if (!bIsSliding)
	{
		Super::CalcVelocity(DeltaTime, Friction, bFluid, BrakingDeceleration);
		return;
	}


	// [슬라이딩 전용 커스텀 물리 연산 시작]

	// 스티어링 (방향 조향)
	// MaxAcceleration을 0으로 만들지 않았으므로, 유저의 입력 방향을 완벽하게 읽어옴
	FVector InputDir = GetCurrentAcceleration().GetSafeNormal();
	if (!InputDir.IsNearlyZero())
	{
		float CurrentSpeed = Velocity.Size2D();
		FVector CurrentDir = Velocity.GetSafeNormal2D();

		// 방향을 부드럽게 꺾어주고, 속도(크기)는 원래대로 유지
		FVector NewDir = FMath::VInterpTo(CurrentDir, InputDir, DeltaTime, SlideSteeringRate).GetSafeNormal();
		Velocity = FVector(NewDir.X * CurrentSpeed, NewDir.Y * CurrentSpeed, Velocity.Z);
	}

	// 2. 지형 판별 및 중력 가속
	FVector FloorNormal = CurrentFloor.HitResult.Normal;
	bool bIsOnSlope = FloorNormal.Z < 0.99f;
	bool bIsGoingDownhill = false;

	float ActualBraking = FlatSlideBraking; // 기본 감속도

	if (bIsOnSlope)
	{
		FVector GravityDir = FVector(0.0f, 0.0f, -1.0f);
		FVector SlopeDown = (GravityDir - FVector::DotProduct(GravityDir, FloorNormal) * FloorNormal).GetSafeNormal();
		bIsGoingDownhill = FVector::DotProduct(Velocity, SlopeDown) > 0.0f;

		if (bIsGoingDownhill)
		{
			// 내리막길: 중력 가속 추가 및 브레이크 최소화
			Velocity += SlopeDown * SlopeGravity * DeltaTime;
			ActualBraking = SlopeSlideBraking;

			// 최고 속도 제한
			if (Velocity.Size2D() > MaxSlopeSlideSpeed)
			{
				Velocity = Velocity.GetSafeNormal2D() * MaxSlopeSlideSpeed + FVector(0, 0, Velocity.Z);
			}
		}
	}

	// 평지일 때의 동적 초과 속도 브레이크 계산
	if (!bIsGoingDownhill)
	{
		float CurrentSpeed = Velocity.Size2D();
		float ExcessSpeed = FMath::Max(0.0f, CurrentSpeed - MinSpeedForHighBraking); 
		ActualBraking = FlatSlideBraking + (ExcessSpeed * HighSpeedBrakingMultiplier);
	}

	// 3. 마찰(브레이크) 수동 적용 
	float Speed2D = Velocity.Size2D();
	if (Speed2D > 0.0f)
	{
		// 감속도를 프레임에 맞춰 빼줌 (0 밑으로 내려가지 않게 Max 처리)
		float NewSpeed = FMath::Max(0.0f, Speed2D - (ActualBraking * DeltaTime));
		Velocity = Velocity.GetSafeNormal2D() * NewSpeed + FVector(0, 0, Velocity.Z);
	}

	// 4. 슬라이딩 자동 종료 조건 (앉기 속도 300 이하로 떨어졌을 때)
	if (Velocity.Size2D() < MaxWalkSpeedCrouched)
	{
		if (CachedBaseChar && CachedBaseChar->IsLocallyControlled())
		{
			OnSlideStateChanged.Broadcast(false);
		}
		StopSlide();
	}
}

/*================================================== 벽넘기 ====================================================*/

bool UBaseMovementComponent::CheckVaulting(FVector& OutTargetLocation, float& OutWallHeight)
{
	if (!CachedBaseChar) return false;

	FVector StartLoc = CachedBaseChar->GetActorLocation();
	FVector ForwardVec = CachedBaseChar->GetActorForwardVector();

	// 1. 앞으로 트레이스 쏘기 (벽 표면 찾기)
	FVector ForwardStart = StartLoc;
	FVector ForwardEnd = StartLoc + (ForwardVec * VaultForwardTraceDistance);

	FHitResult ForwardHit;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(CachedBaseChar);

	bool bHitWall = GetWorld()->LineTraceSingleByChannel(ForwardHit, ForwardStart, ForwardEnd, ECC_Visibility, QueryParams);

	// 수직에 가까운 벽이 아니면 실패
	if (!bHitWall || FMath::Abs(ForwardHit.Normal.Z) > 0.2f) {
		return false;
	}

	// 2. 아래로 트레이스 쏘기 (벽 꼭대기 찾기)
	FVector DownwardStart = ForwardHit.Location + (ForwardVec * VaultDepthOffsetMargin) + FVector(0, 0, VaultUpwardTraceHeight);
	FVector DownwardEnd = DownwardStart - FVector(0, 0, VaultUpwardTraceHeight * 1.5f);

	FHitResult DownwardHit;
	// 캡슐 트레이스로 쏴서 캐릭터가 설 수 있는 충분한 공간이 있는지 확인
	bool bHitTop = GetWorld()->SweepSingleByChannel(
		DownwardHit, DownwardStart, DownwardEnd, FQuat::Identity, ECC_Visibility,
		FCollisionShape::MakeCapsule(CachedBaseChar->GetSimpleCollisionRadius(), CachedBaseChar->GetSimpleCollisionHalfHeight()), QueryParams);

	if (bHitTop && DownwardHit.Normal.Z > 0.8f) // 평평한 바닥인가?
	{
		// 3. 벽 높이 계산 및 통과
		float FootZ = StartLoc.Z - CachedBaseChar->GetSimpleCollisionHalfHeight();
		OutWallHeight = DownwardHit.Location.Z - FootZ;

		// 너무 낮거나(계단), 키의 1.5배(VaultUpwardTraceHeight)를 초과하면 실패
		if (OutWallHeight < VaultMinHeight || OutWallHeight > VaultUpwardTraceHeight) {
			UE_LOG(LogTemp, Warning, TEXT("낮거나 높음"));
			return false;
		}

		// 벽 안쪽으로 얼마나 들어갈꺼냐 
		float VaultDepthOffset = CachedBaseChar->GetSimpleCollisionRadius() + VaultDepthOffsetMargin; 
		FVector WallEdgeXY = ForwardHit.Location + (ForwardVec * VaultDepthOffset);

		// 최종 착지 위치 (캡슐 절반 높이 + 미세 보정치 적용하여 끼임 방지)
		float ZOffset = CachedBaseChar->GetSimpleCollisionHalfHeight() + VaultZOffsetMargin;
		OutTargetLocation = FVector(WallEdgeXY.X, WallEdgeXY.Y, DownwardHit.Location.Z + ZOffset);

		return true;
	}

	return false;
}

