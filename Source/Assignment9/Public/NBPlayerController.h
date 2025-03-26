// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "NBPlayerController.generated.h"

/**
* 위젯 표시
* - 채팅창
* - 턴 정보
* 
* 메시지를 서버에 전송
* 결과에 따라 승리, 패배, 계속 진행
*/
class UNBChatWindow;
UCLASS()
class ASSIGNMENT9_API ANBPlayerController : public APlayerController
{
	GENERATED_BODY()

private:
	// 채팅창 위젯 클래스
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UUserWidget> ChatWindowWidgetClass;
	// 채팅창 위젯 인스턴스
	UUserWidget* ChatWindowWidgetInstance;

public:
	// 생성자
	ANBPlayerController();

	// 채팅창 위젯을 반환하는 함수
	UUserWidget* GetChatWindowWidget() const;

	// 채팅창 위젯을 보여주는 함수
	void ShowChatWindowWidget();

	// 위젯에 현재 턴 정보를 업데이트하는 함수
	UFUNCTION(Client, Reliable)
	void ClientUpdateChatWindowWidget();

public:
	// 서버에 채팅 메시지를 전송하는 함수, GameMode에서 비교를 진행
	UFUNCTION(Server, Reliable)
	void ServerSendChatMessage(const FString& Message);
	
	// 비교 결과에 따라 승리처리를 하는 함수
	UFUNCTION(Server, Reliable)
	void ServerWin();

	// 비교 결과에 따라 패배처리를 하는 함수
	UFUNCTION(Server, Reliable)
	void ServerDefeat();

	// 비교 결과에 따라 계속 진행을 하는 함수
	UFUNCTION(Server, Reliable)
	void ServerContinue();

	// 클라이언트에서 승리/패배 UI표시하는 함수
	UFUNCTION(NetMulticast, Reliable)
	void MulticastShowGameResult(bool bIsWinner);

	// 클라이언트에서 턴 변경 UI표시하는 함수
	UFUNCTION(Client, Reliable)
	void ClientShowGameContinue();

	// 게임 종료후 재개를 위한 리셋 함수
	UFUNCTION(Server, Reliable)
	void ServerResetGame();

public:
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 서버에 UserID를 요청하는 함수
	UFUNCTION(Server, Reliable)
	void ServerRequestUserID();

	// 서버에서 GoalNumber를 설정하는 함수
	UFUNCTION(Server, Reliable)
	void ServerSetupGoalNumber();

	// 서버가 정한 턴을 반영
	UFUNCTION(Client, Reliable)
	void ClientReceiveTurnOrder(bool bIsFirst);

	UFUNCTION(Server, Reliable)
	void ServerRequestCurrentTurnPlayer();

	// 턴 순서에 해당하는 플레이어의 UserID
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly)
	FString CurrentTurnPlayerID;

	UFUNCTION(Client, Reliable)
	void ClientUpdateCurrentTurnPlayer(APlayerController* NewTurnPlayer);
};
