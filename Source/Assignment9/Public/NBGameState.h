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

    UFUNCTION(NetMulticast, Reliable)
    void MulticastShowGameResult(const FString& UserID);

private:
	// GetLifetimeReplicatedProps �Լ��� �������̵��Ͽ� ChatMessages�� GoalNumber�� Replicate�Ѵ�.
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    
    // GoalNumber�� ����Ǹ� ȣ��Ǵ� �Լ�
    UFUNCTION()
    void OnRep_GoalNumber();
	// GameOver ���ΰ� ����Ǹ� ȣ��Ǵ� �Լ�
	UFUNCTION()
	void OnRep_bGameOver();
    UFUNCTION()
    void OnRep_CurrentTurnPlayer();
};
