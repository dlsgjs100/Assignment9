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

	// ä�� �޽����� �����ϴ� �迭
    UPROPERTY(ReplicatedUsing = OnRep_ChatMessages)
    TArray<FString> ChatMessages;
	// GoalNumber
    UPROPERTY(ReplicatedUsing = OnRep_GoalNumber)
    FString GoalNumber;
	// GameOver ����
    UPROPERTY(ReplicatedUsing = OnRep_bGameOver)
    bool bGameOver;
    // ���� ���� �÷��̾�
    UPROPERTY(ReplicatedUsing = OnRep_CurrentTurnPlayer)
    APlayerController* CurrentTurnPlayer;

   
    
    
    // �� ��ȯ �Լ�
    UFUNCTION()
    void SwitchTurn(); 

private:
	// GetLifetimeReplicatedProps �Լ��� �������̵��Ͽ� ChatMessages�� GoalNumber�� Replicate�Ѵ�.
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    
    // GoalNumber�� ����Ǹ� ȣ��Ǵ� �Լ�
    UFUNCTION()
    void OnRep_ChatMessages();
	// ä�� �޽����� ����Ǹ� ȣ��Ǵ� �Լ�
    UFUNCTION()
    void OnRep_GoalNumber();
	// GameOver ���ΰ� ����Ǹ� ȣ��Ǵ� �Լ�
	UFUNCTION()
	void OnRep_bGameOver();
    UFUNCTION()
    void OnRep_CurrentTurnPlayer();

public:
    // ��� Ŭ���̾�Ʈ���� ä�� �޽����� �����ϴ� �Լ�
    UFUNCTION(NetMulticast, Reliable)
    void MulticastChatMessage(const FString& NewMessage);
};
