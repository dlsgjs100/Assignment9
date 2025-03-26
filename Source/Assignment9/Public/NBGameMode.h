// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "NBGameMode.generated.h"

/**
 * UserID와 TurnOrder를 설정
 * GoalNumber를 설정, 비교
 * 비교 결과를 클라이언트로 전송
 */
class ANBPlayerController;
UCLASS()
class ASSIGNMENT9_API ANBGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	ANBGameMode();

	// 승리, 패배, 계속진행을 PlayerState에서 변수로 저장해 추적하는 것이 옳은가
	// GameMode에서는 결과 문자열만 반환하고 각각 PlayerController에서 처리하는 것이 옳은가
	FString Result;

public:
	// GoalNumber를 설정하는 함수
	void SetupGoalNumber();

	// Message와 GoalNumber와 비교하는 함수
	void CompareMessagetoGoalNumber(const FString& Message, ANBPlayerController* NBPlayerController);

	// 클라이언트의 UserID를 설정하는 함수
	UFUNCTION(Client, Reliable)
	void ClientSetUserID(ANBPlayerController* NBPlayerController);
	// 서버에서 TurnOrder를 설정하는 함수
	UFUNCTION(Server, Reliable)
	void ServerSetTurnOrder();

	FTimerHandle TurnOrderRetryTimer;

};
