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

	// 로컬 플레이어인 경우 채팅창 위젯을 보여준다.
	if (IsLocalController())
	{
		ShowChatWindowWidget();
		ServerSetupGoalNumber();
	}

	// 서버인 경우 GoalNumber를 설정한다.
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
		// 위젯 생성 시 확인
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

		// 현재 게임 상태 가져오기
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

		// FText로 변환 후 설정
		ScoreBoardText->SetText(FText::FromString(ScoreBoardString));
	}
}

void ANBPlayerController::ServerSendChatMessage_Implementation(const FString& Message)
{
	if (Message.IsEmpty())
	{
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
}

void ANBPlayerController::ServerWin_Implementation()
{
	AGameStateBase* GameState = GetWorld()->GetGameState();
	ANBGameState* NBGameState = Cast<ANBGameState>(GameState);
	ANBPlayerState* NBPlayerState = GetPlayerState<ANBPlayerState>();

	if (!NBGameState || !NBPlayerState) return;

	UNBGameRulesLibrary::WinGame(NBPlayerState->GetUserID(), NBGameState->bGameOver);
	NBPlayerState->SetPlayerScore(NBPlayerState->GetPlayerScore() + 1);

	// GameState에서 턴을 변경
	NBGameState->SwitchTurn();

	// 모든 클라이언트에게 승리 메시지 전달
	NBGameState->MulticastShowGameResult(NBPlayerState->GetUserID());

	// 게임 상태 초기화
	ServerResetGame();
}

void ANBPlayerController::ServerDefeat_Implementation()
{
	AGameStateBase* GameState = GetWorld()->GetGameState();
	ANBGameState* NBGameState = Cast<ANBGameState>(GameState);
	ANBPlayerState* NBPlayerState = GetPlayerState<ANBPlayerState>();

	if (!NBGameState || !NBPlayerState) return;

	UNBGameRulesLibrary::DefeatGame(NBPlayerState->GetUserID(), NBGameState->bGameOver);

	// GameState에서 턴을 변경
	NBGameState->SwitchTurn();

	// 게임 상태 초기화
	ServerResetGame();
}

void ANBPlayerController::ServerContinue_Implementation()
{
	ANBGameState* NBGameState = GetWorld()->GetGameState<ANBGameState>();
	ANBPlayerState* NBPlayerState = GetPlayerState<ANBPlayerState>();

	if (!NBGameState || !NBPlayerState) return;

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

	// 모든 플레이어의 PlayerChance가 0이면 게임을 리셋
	if (bAllPlayerChanceZero)
	{
		ServerResetGame();
	}

	// GameState에서 턴을 변경
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
			// UserID를 가져와서 CurrentTurnPlayerID에 설정
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