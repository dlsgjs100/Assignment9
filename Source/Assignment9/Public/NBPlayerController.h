// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "NBPlayerController.generated.h"

/**
* ���� ǥ��
* - ä��â
* - �� ����
* 
* �޽����� ������ ����
* ����� ���� �¸�, �й�, ��� ����
*/
class UNBChatWindow;
UCLASS()
class ASSIGNMENT9_API ANBPlayerController : public APlayerController
{
	GENERATED_BODY()

private:
	// ä��â ���� Ŭ����
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UUserWidget> ChatWindowWidgetClass;
	// ä��â ���� �ν��Ͻ�
	UUserWidget* ChatWindowWidgetInstance;

public:
	// ������
	ANBPlayerController();

	// ä��â ������ ��ȯ�ϴ� �Լ�
	UUserWidget* GetChatWindowWidget() const;

	// ä��â ������ �����ִ� �Լ�
	void ShowChatWindowWidget();

	// ������ ���� �� ������ ������Ʈ�ϴ� �Լ�
	UFUNCTION(Client, Reliable)
	void ClientUpdateChatWindowWidget();

public:
	// ������ ä�� �޽����� �����ϴ� �Լ�, GameMode���� �񱳸� ����
	UFUNCTION(Server, Reliable)
	void ServerSendChatMessage(const FString& Message);
	
	// �� ����� ���� �¸�ó���� �ϴ� �Լ�
	UFUNCTION(Server, Reliable)
	void ServerWin();

	// �� ����� ���� �й�ó���� �ϴ� �Լ�
	UFUNCTION(Server, Reliable)
	void ServerDefeat();

	// �� ����� ���� ��� ������ �ϴ� �Լ�
	UFUNCTION(Server, Reliable)
	void ServerContinue();

	// Ŭ���̾�Ʈ���� �¸�/�й� UIǥ���ϴ� �Լ�
	UFUNCTION(NetMulticast, Reliable)
	void MulticastShowGameResult(bool bIsWinner);

	// Ŭ���̾�Ʈ���� �� ���� UIǥ���ϴ� �Լ�
	UFUNCTION(Client, Reliable)
	void ClientShowGameContinue();

	// ���� ������ �簳�� ���� ���� �Լ�
	UFUNCTION(Server, Reliable)
	void ServerResetGame();

public:
	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// ������ UserID�� ��û�ϴ� �Լ�
	UFUNCTION(Server, Reliable)
	void ServerRequestUserID();

	// �������� GoalNumber�� �����ϴ� �Լ�
	UFUNCTION(Server, Reliable)
	void ServerSetupGoalNumber();

	// ������ ���� ���� �ݿ�
	UFUNCTION(Client, Reliable)
	void ClientReceiveTurnOrder(bool bIsFirst);

	UFUNCTION(Server, Reliable)
	void ServerRequestCurrentTurnPlayer();

	// �� ������ �ش��ϴ� �÷��̾��� UserID
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly)
	FString CurrentTurnPlayerID;

	UFUNCTION(Client, Reliable)
	void ClientUpdateCurrentTurnPlayer(APlayerController* NewTurnPlayer);
};
