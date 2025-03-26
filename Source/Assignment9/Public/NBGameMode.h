// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "NBGameMode.generated.h"

/**
 * UserID�� TurnOrder�� ����
 * GoalNumber�� ����, ��
 * �� ����� Ŭ���̾�Ʈ�� ����
 */
class ANBPlayerController;
UCLASS()
class ASSIGNMENT9_API ANBGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	ANBGameMode();

	// �¸�, �й�, ��������� PlayerState���� ������ ������ �����ϴ� ���� ������
	// GameMode������ ��� ���ڿ��� ��ȯ�ϰ� ���� PlayerController���� ó���ϴ� ���� ������
	FString Result;

public:
	// GoalNumber�� �����ϴ� �Լ�
	void SetupGoalNumber();

	// Message�� GoalNumber�� ���ϴ� �Լ�
	void CompareMessagetoGoalNumber(const FString& Message, ANBPlayerController* NBPlayerController);

	// Ŭ���̾�Ʈ�� UserID�� �����ϴ� �Լ�
	UFUNCTION(Client, Reliable)
	void ClientSetUserID(ANBPlayerController* NBPlayerController);
	// �������� TurnOrder�� �����ϴ� �Լ�
	UFUNCTION(Server, Reliable)
	void ServerSetTurnOrder();

	FTimerHandle TurnOrderRetryTimer;

};
