// Copyright Epic Games, Inc. All Rights Reserved.

#include "NBPlayerController.h"
#include "NBChatWindow.h"
#include "NBGameMode.h"
#include "NBGameState.h"
#include "NBPlayerState.h"
#include "NBGenerateRandomNumberLibrary.h"
#include "NBGameRulesLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "EngineUtils.h"

ANBPlayerController::ANBPlayerController()
	:ChatWindowWidgetClass(nullptr),
	ChatWindowWidgetInstance(nullptr)
{
}

void ANBPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// ���� �÷��̾��� ��� ä��â ������ �����ش�.
	if (IsLocalController())
	{
		ShowChatWindowWidget();
	}

	// ������ ��� GoalNumber�� �����ϰ�, UserID�� ��û�Ѵ�.
	if (HasAuthority())
	{
		ServerSetupGoalNumber();
		ServerRequestUserID();
		ClientUpdateCurrentTurnPlayer(this);
	}
}

UUserWidget* ANBPlayerController::GetChatWindowWidget() const
{
	return ChatWindowWidgetInstance;
}

void ANBPlayerController::ShowChatWindowWidget()
{
	UE_LOG(LogTemp, Warning, TEXT("ShowChatWindowWidget called"));

	if (ChatWindowWidgetInstance)
	{
		ChatWindowWidgetInstance->RemoveFromParent();
		ChatWindowWidgetInstance = nullptr;
	}

	if (ChatWindowWidgetClass)
	{
		// ���� ���� �� Ȯ��
		ChatWindowWidgetInstance = CreateWidget<UUserWidget>(this, ChatWindowWidgetClass);

		if (ChatWindowWidgetInstance == nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to create ChatWindowWidgetInstance"));
			return;
		}

		ChatWindowWidgetInstance->AddToViewport();
		if (UNBChatWindow* ChatWindow = Cast<UNBChatWindow>(ChatWindowWidgetInstance))
		{
			ChatWindow->OnUserInputCommitted.AddDynamic(this, &ANBPlayerController::ServerSendChatMessage);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("ChatWindowWidgetClass is not set"));
	}
}

// ������ ���� �� ������ ������Ʈ�ϴ� �Լ�
void ANBPlayerController::ClientUpdateChatWindowWidget_Implementation()
{
	ANBPlayerState* NBPlayerState = GetPlayerState<ANBPlayerState>();
	// GetChatWindowWidget()�� ��ȿ���� Ȯ��
	UUserWidget* ChatWindowWidget = GetChatWindowWidget();
	if (!ChatWindowWidget)
	{
		return;  // ������ ���ٸ� �Լ� ����
	}

	UImage* IsMyTurn = Cast<UImage>(GetChatWindowWidget()->GetWidgetFromName(TEXT("IsMyTurn")));

	// IsMyTurn ������ ��ȿ���� Ȯ��
	if (IsMyTurn)
	{
		// �÷��̾� ���¸� Ȯ���Ͽ� ���� ǥ��
		if (NBPlayerState && NBPlayerState->GetIsMyTurn())
		{
			UE_LOG(LogTemp, Warning, TEXT("It is my turn!"));
			IsMyTurn->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("It is not my turn!"));
			IsMyTurn->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}

void ANBPlayerController::ServerSendChatMessage_Implementation(const FString& Message)
{
	if (Message.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("Message is empty!"));
		return;
	}

	// �� ���ʰ� �ƴ� ��� �޽��� �������� ����
	ANBPlayerState* NBPlayerState = GetPlayerState<ANBPlayerState>();
	if (NBPlayerState && !NBPlayerState->GetIsMyTurn()) // bIsMyTurn�� false�� ���� "It's not my turn!"
	{
		return;
	}

	// GameMode���� �޽����� ��ǥ ���� ��
	ANBGameMode* NBGameMode = GetWorld()->GetAuthGameMode<ANBGameMode>();
	if (NBGameMode)
	{
		NBGameMode->CompareMessagetoGoalNumber(Message, this);
	}

	// ��Ƽĳ��Ʈ�� �޽��� ����
	ANBGameState* NBGameState = GetWorld()->GetGameState<ANBGameState>();
	if (NBGameState)
	{
		NBGameState->MulticastChatMessage(Message);
	}
}

void ANBPlayerController::ServerWin_Implementation()
{
	AGameStateBase* GameState = GetWorld()->GetGameState();
	ANBGameState* NBGameState = Cast<ANBGameState>(GameState);
	ANBPlayerState* NBPlayerState = GetPlayerState<ANBPlayerState>();

	if (!NBGameState || !NBPlayerState) return;

	UE_LOG(LogTemp, Warning, TEXT("ServerWin called! : %s"), *NBPlayerState->GetUserID());

	UNBGameRulesLibrary::WinGame(NBPlayerState->GetUserID(), NBGameState->bGameOver);
	NBPlayerState->SetScore(NBPlayerState->GetScore() + 1);

	// GameState���� ���� ����
	NBGameState->SwitchTurn();

	// ��� Ŭ���̾�Ʈ���� �¸� �޽��� ����
	MulticastShowGameResult(true);

	// ���� ���� �ʱ�ȭ
	ServerResetGame();
}

void ANBPlayerController::ServerDefeat_Implementation()
{
	AGameStateBase* GameState = GetWorld()->GetGameState();
	ANBGameState* NBGameState = Cast<ANBGameState>(GameState);
	ANBPlayerState* NBPlayerState = GetPlayerState<ANBPlayerState>();

	if (!NBGameState || !NBPlayerState) return;

	UE_LOG(LogTemp, Warning, TEXT("ServerDefeat called! : %s"), *NBPlayerState->GetUserID());

	UNBGameRulesLibrary::DefeatGame(NBPlayerState->GetUserID(), NBGameState->bGameOver);

	// GameState���� ���� ����
	NBGameState->SwitchTurn();

	// ��� Ŭ���̾�Ʈ���� �й� �޽��� ����
	MulticastShowGameResult(false);

	// ���� ���� �ʱ�ȭ
	ServerResetGame();
}

void ANBPlayerController::ServerContinue_Implementation()
{
	ANBGameState* NBGameState = GetWorld()->GetGameState<ANBGameState>();
	ANBPlayerState* NBPlayerState = GetPlayerState<ANBPlayerState>();

	if (!NBGameState || !NBPlayerState) return;

	UE_LOG(LogTemp, Warning, TEXT("ServerContinue called! : %s"), *NBPlayerState->GetUserID());

	UNBGameRulesLibrary::DecreaseChances(NBPlayerState->GetPlayerChance());

	// ��� �÷��̾��� PlayerChance�� 0���� Ȯ��
	bool bAllPlayerChanceZero = true;
	for (TActorIterator<APlayerController> It(GetWorld()); It; ++It)
	{
		ANBPlayerController* NBPlayerController = Cast<ANBPlayerController>(*It);
		if (NBPlayerController)
		{
			ANBPlayerState* CurrentNBPlayerState = NBPlayerController->GetPlayerState<ANBPlayerState>();
			if (CurrentNBPlayerState && CurrentNBPlayerState->GetPlayerChance() > 0)
			{
				bAllPlayerChanceZero = false;
				break;  // �ϳ��� PlayerChance�� 0�� �ƴϸ� ����
			}
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("bAllPlayerChanceZero is %s!"), bAllPlayerChanceZero ? TEXT("TRUE") : TEXT("FALSE"));

	// ��� �÷��̾��� PlayerChance�� 0�̸� ������ ����
	if (bAllPlayerChanceZero)
	{
		ServerResetGame();
	}

	// GameState���� ���� ����
	NBGameState->SwitchTurn();

	// Ŭ���̾�Ʈ���� UI ������Ʈ
	ClientShowGameContinue();
}

void ANBPlayerController::MulticastShowGameResult_Implementation(bool bIsWinner)
{
	if (bIsWinner)
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "WIN");
		// ShowWinUI();
	}
	else
	{
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, "LOSE");
		// ShowLoseUI();
	}
}

void ANBPlayerController::ClientShowGameContinue_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("Continue!"));
	// ShowContinueUI();
}

void ANBPlayerController::ServerResetGame_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("ServerResetGame called!"));

	ANBGameState* NBGameState = GetWorld()->GetGameState<ANBGameState>();
	ANBGameMode* NBGameMode = GetWorld()->GetAuthGameMode<ANBGameMode>();
	if (NBGameState)
	{
		NBGameState->ChatMessages.Empty();
		NBGameState->bGameOver = false;
	}
	if (NBGameMode)
	{
		NBGameMode->SetupGoalNumber();
	}

	for (TActorIterator<APlayerController> It(GetWorld()); It; ++It)
	{
		ANBPlayerController* PlayerController = Cast<ANBPlayerController>(*It);
		if (PlayerController)
		{
			ANBPlayerState* NBPlayerState = PlayerController->GetPlayerState<ANBPlayerState>();
			NBPlayerState->SetPlayerChance(NBPlayerState->DefaultPlayerChance);
		}
	}
}

void ANBPlayerController::ServerRequestUserID_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("ServerRequestUserID called!"));
	ANBGameMode* NBGameMode = GetWorld()->GetAuthGameMode<ANBGameMode>();
	if (NBGameMode)
	{
		NBGameMode->ClientSetUserID(this);
		NBGameMode->ServerSetTurnOrder();
	}
}

void ANBPlayerController::ServerSetupGoalNumber_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("ServerSetupGoalNumber called!"));
	ANBGameMode* NBGameMode = GetWorld()->GetAuthGameMode<ANBGameMode>();
	if (NBGameMode)
	{
		NBGameMode->SetupGoalNumber();
	}
}

void ANBPlayerController::ClientReceiveTurnOrder_Implementation(bool bIsFirst)
{
	ANBPlayerState* NBPS = GetPlayerState<ANBPlayerState>();
	if (NBPS)
	{
		NBPS->SetIsMyTurn(bIsFirst);

		UE_LOG(LogTemp, Warning, TEXT("Client %s received turn: %s"),
			*NBPS->GetUserID(),
			bIsFirst ? TEXT("first") : TEXT("second"));

		ANBGameState* NBGameState = GetWorld()->GetGameState<ANBGameState>();
		if (NBGameState)
		{
			if (bIsFirst)
			{
				NBGameState->CurrentTurnPlayer = NBPS->GetPlayerController();
			}
		}
	}
}

void ANBPlayerController::ServerRequestCurrentTurnPlayer_Implementation()
{
	ANBGameState* GS = GetWorld()->GetGameState<ANBGameState>();
	if (GS && GS->CurrentTurnPlayer)
	{
		ClientUpdateCurrentTurnPlayer(GS->CurrentTurnPlayer);
	}
}

void ANBPlayerController::ClientUpdateCurrentTurnPlayer_Implementation(APlayerController* NewTurnPlayer)
{
	if (NewTurnPlayer)
	{
		ANBPlayerState* NBPlayerState = NewTurnPlayer->GetPlayerState<ANBPlayerState>();
		if (NBPlayerState)
		{
			// UserID�� �����ͼ� CurrentTurnPlayerID�� ����
			CurrentTurnPlayerID = NBPlayerState->GetUserID();
		}
	}
}

void ANBPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ANBPlayerController, CurrentTurnPlayerID);
}