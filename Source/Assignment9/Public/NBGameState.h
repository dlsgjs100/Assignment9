// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "NBGameState.generated.h"

/**
 *
 */
UCLASS()
class ASSIGNMENT9_API ANBGameState : public AGameStateBase
{
    GENERATED_BODY()

public:
    ANBGameState();

    void SetGoalNumber();

	// 채팅 메시지를 저장하는 배열
    UPROPERTY(ReplicatedUsing = OnRep_ChatMessages)
    TArray<FString> ChatMessages;
	// GoalNumber
    UPROPERTY(ReplicatedUsing = OnRep_GoalNumber)
    FString GoalNumber;
	// GameOver 여부
    UPROPERTY(ReplicatedUsing = OnRep_bGameOver)
    bool bGameOver;
    // 현재 턴인 플레이어
    UPROPERTY(ReplicatedUsing = OnRep_CurrentTurnPlayer)
    APlayerController* CurrentTurnPlayer;

   
    
    
    // 턴 전환 함수
    UFUNCTION()
    void SwitchTurn(); 

private:
	// GetLifetimeReplicatedProps 함수를 오버라이드하여 ChatMessages와 GoalNumber를 Replicate한다.
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    
    // GoalNumber가 변경되면 호출되는 함수
    UFUNCTION()
    void OnRep_ChatMessages();
	// 채팅 메시지가 변경되면 호출되는 함수
    UFUNCTION()
    void OnRep_GoalNumber();
	// GameOver 여부가 변경되면 호출되는 함수
	UFUNCTION()
	void OnRep_bGameOver();
    UFUNCTION()
    void OnRep_CurrentTurnPlayer();

public:
    // 모든 클라이언트에게 채팅 메시지를 전송하는 함수
    UFUNCTION(NetMulticast, Reliable)
    void MulticastChatMessage(const FString& NewMessage);
};
