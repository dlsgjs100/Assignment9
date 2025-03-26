// Copyright Epic Games, Inc. All Rights Reserved.

#include "NBPlayerController.h"
#include "NBChatWindow.h"
#include "NBGameMode.h"
#include "NBGameState.h"
#include "NBPlayerState.h"
#include "NBGenerateRandomNumberLibrary.h"
#include "NBGameRulesLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "EngineUtils.h"

ANBPlayerController::ANBPlayerController()
	:ChatWindowWidgetClass(nullptr),
	ChatWindowWidgetInstance(nullptr)
{
}

void ANBPlayerController::BeginPlay()
{
	Super::BeginPlay();

	FInputModeUIOnly InputModeUIOnly;
	SetInputMode(InputModeUIOnly);

	// ���� �÷��̾��� ��� ä��â ������ �����ش�.
	if (IsLocalController())
	{
		ShowChatWindowWidget();
		ServerSetupGoalNumber();
	}

	// ������ ��� GoalNumber�� �����Ѵ�.
	if (GetLocalRole() == ROLE_Authority)
	{
		ServerRequestUserID();
		ServerRequestCurrentTurnPlayer();
	}
}

UUserWidget* ANBPlayerController::GetChatWindowWidget() const
{
	return ChatWindowWidgetInstance;
}

void ANBPlayerController::ShowChatWindowWidget()
{
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
			return;
		}

		ChatWindowWidgetInstance->AddToViewport();
		if (UNBChatWindow* ChatWindow = Cast<UNBChatWindow>(ChatWindowWidgetInstance))
		{
			ChatWindow->OnUserInputCommitted.AddDynamic(this, &ANBPlayerController::ServerSendChatMessage);
		}
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
			IsMyTurn->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			IsMyTurn->SetVisibility(ESlateVisibility::Hidden);
		}
	}

	UTextBlock* ScoreBoardText = Cast<UTextBlock>(GetChatWindowWidget()->GetWidgetFromName(TEXT("ScoreBoardText")));
	if (ScoreBoardText)
	{
		FString ScoreBoardString;

		// ���� ���� ���� ��������
		ANBGameState* NBGameState = Cast<ANBGameState>(GetWorld()->GetGameState());
		if (NBGameState)
		{
			for (APlayerState * PS : NBGameState->PlayerArray)
			{
				ANBPlayerState* NBPS = Cast<ANBPlayerState>(PS);
				if (NBPS)
				{
					FString PlayerName = NBPS->UserID.IsEmpty() ? TEXT("Unknown") : NBPS->UserID;
					FString PlayerEntry = FString::Printf(TEXT("%s : %d"), *PlayerName, NBPS->GetPlayerScore());
					ScoreBoardString += PlayerEntry + TEXT("\n");
				}
			}
		}

		// FText�� ��ȯ �� ����
		ScoreBoardText->SetText(FText::FromString(ScoreBoardString));
	}
}

void ANBPlayerController::ServerSendChatMessage_Implementation(const FString& Message)
{
	if (Message.IsEmpty())
	{
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
}

void ANBPlayerController::ServerWin_Implementation()
{
	AGameStateBase* GameState = GetWorld()->GetGameState();
	ANBGameState* NBGameState = Cast<ANBGameState>(GameState);
	ANBPlayerState* NBPlayerState = GetPlayerState<ANBPlayerState>();

	if (!NBGameState || !NBPlayerState) return;

	UNBGameRulesLibrary::WinGame(NBPlayerState->GetUserID(), NBGameState->bGameOver);
	NBPlayerState->SetPlayerScore(NBPlayerState->GetPlayerScore() + 1);

	// GameState���� ���� ����
	NBGameState->SwitchTurn();

	// ��� Ŭ���̾�Ʈ���� �¸� �޽��� ����
	NBGameState->MulticastShowGameResult(NBPlayerState->GetUserID());

	// ���� ���� �ʱ�ȭ
	ServerResetGame();
}

void ANBPlayerController::ServerDefeat_Implementation()
{
	AGameStateBase* GameState = GetWorld()->GetGameState();
	ANBGameState* NBGameState = Cast<ANBGameState>(GameState);
	ANBPlayerState* NBPlayerState = GetPlayerState<ANBPlayerState>();

	if (!NBGameState || !NBPlayerState) return;

	UNBGameRulesLibrary::DefeatGame(NBPlayerState->GetUserID(), NBGameState->bGameOver);

	// GameState���� ���� ����
	NBGameState->SwitchTurn();

	// ���� ���� �ʱ�ȭ
	ServerResetGame();
}

void ANBPlayerController::ServerContinue_Implementation()
{
	ANBGameState* NBGameState = GetWorld()->GetGameState<ANBGameState>();
	ANBPlayerState* NBPlayerState = GetPlayerState<ANBPlayerState>();

	if (!NBGameState || !NBPlayerState) return;

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

	// ��� �÷��̾��� PlayerChance�� 0�̸� ������ ����
	if (bAllPlayerChanceZero)
	{
		ServerResetGame();
	}

	// GameState���� ���� ����
	NBGameState->SwitchTurn();

}

void ANBPlayerController::ServerResetGame_Implementation()
{
	ANBGameState* NBGameState = GetWorld()->GetGameState<ANBGameState>();
	ANBGameMode* NBGameMode = GetWorld()->GetAuthGameMode<ANBGameMode>();
	if (NBGameState)
	{
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
	AGameModeBase* GameMode = UGameplayStatics::GetGameMode(this);
	if (IsValid(GameMode) == true)
	{
		ANBGameMode* NBGameMode = Cast<ANBGameMode>(GameMode);
		if (IsValid(NBGameMode) == true)
		{
			NBGameMode->ClientSetUserID(this);
			NBGameMode->ServerSetTurnOrder();
		}
	}
}

void ANBPlayerController::ServerSetupGoalNumber_Implementation()
{
	AGameModeBase* GameMode = UGameplayStatics::GetGameMode(this);
	if (IsValid(GameMode) == true)
	{
		ANBGameMode* NBGameMode = Cast<ANBGameMode>(GameMode);
		if (IsValid(NBGameMode) == true)
		{
			NBGameMode->SetupGoalNumber();
		}
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

	ClientUpdateChatWindowWidget();
}

void ANBPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ANBPlayerController, CurrentTurnPlayerID);
}