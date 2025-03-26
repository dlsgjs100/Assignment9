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

	// 로컬 플레이어인 경우 채팅창 위젯을 보여준다.
	if (IsLocalController())
	{
		ShowChatWindowWidget();
	}

	// 서버인 경우 GoalNumber를 설정하고, UserID를 요청한다.
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
		// 위젯 생성 시 확인
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

// 위젯에 현재 턴 정보를 업데이트하는 함수
void ANBPlayerController::ClientUpdateChatWindowWidget_Implementation()
{
	ANBPlayerState* NBPlayerState = GetPlayerState<ANBPlayerState>();
	// GetChatWindowWidget()가 유효한지 확인
	UUserWidget* ChatWindowWidget = GetChatWindowWidget();
	if (!ChatWindowWidget)
	{
		return;  // 위젯이 없다면 함수 종료
	}

	UImage* IsMyTurn = Cast<UImage>(GetChatWindowWidget()->GetWidgetFromName(TEXT("IsMyTurn")));

	// IsMyTurn 위젯이 유효한지 확인
	if (IsMyTurn)
	{
		// 플레이어 상태를 확인하여 턴을 표시
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

	// 내 차례가 아닌 경우 메시지 전송하지 않음
	ANBPlayerState* NBPlayerState = GetPlayerState<ANBPlayerState>();
	if (NBPlayerState && !NBPlayerState->GetIsMyTurn()) // bIsMyTurn이 false일 때만 "It's not my turn!"
	{
		return;
	}

	// GameMode에서 메시지와 목표 숫자 비교
	ANBGameMode* NBGameMode = GetWorld()->GetAuthGameMode<ANBGameMode>();
	if (NBGameMode)
	{
		NBGameMode->CompareMessagetoGoalNumber(Message, this);
	}

	// 멀티캐스트로 메시지 전파
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

	// GameState에서 턴을 변경
	NBGameState->SwitchTurn();

	// 모든 클라이언트에게 승리 메시지 전달
	MulticastShowGameResult(true);

	// 게임 상태 초기화
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

	// GameState에서 턴을 변경
	NBGameState->SwitchTurn();

	// 모든 클라이언트에게 패배 메시지 전달
	MulticastShowGameResult(false);

	// 게임 상태 초기화
	ServerResetGame();
}

void ANBPlayerController::ServerContinue_Implementation()
{
	ANBGameState* NBGameState = GetWorld()->GetGameState<ANBGameState>();
	ANBPlayerState* NBPlayerState = GetPlayerState<ANBPlayerState>();

	if (!NBGameState || !NBPlayerState) return;

	UE_LOG(LogTemp, Warning, TEXT("ServerContinue called! : %s"), *NBPlayerState->GetUserID());

	UNBGameRulesLibrary::DecreaseChances(NBPlayerState->GetPlayerChance());

	// 모든 플레이어의 PlayerChance가 0인지 확인
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
				break;  // 하나라도 PlayerChance가 0이 아니면 종료
			}
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("bAllPlayerChanceZero is %s!"), bAllPlayerChanceZero ? TEXT("TRUE") : TEXT("FALSE"));

	// 모든 플레이어의 PlayerChance가 0이면 게임을 리셋
	if (bAllPlayerChanceZero)
	{
		ServerResetGame();
	}

	// GameState에서 턴을 변경
	NBGameState->SwitchTurn();

	// 클라이언트에서 UI 업데이트
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
			// UserID를 가져와서 CurrentTurnPlayerID에 설정
			CurrentTurnPlayerID = NBPlayerState->GetUserID();
		}
	}
}

void ANBPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ANBPlayerController, CurrentTurnPlayerID);
}