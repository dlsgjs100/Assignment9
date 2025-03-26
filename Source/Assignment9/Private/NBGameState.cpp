// Copyright Epic Games, Inc. All Rights Reserved.

#include "NBGameState.h"
#include "NBPlayerController.h"
#include "NBPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "EngineUtils.h" 
#include "Components/TextBlock.h"
#include "Blueprint/UserWidget.h"
#include "NBGenerateRandomNumberLibrary.h"

ANBGameState::ANBGameState()
{
	bReplicates = true;
	GoalNumber = TEXT("");
	bGameOver = false;
}

void ANBGameState::SetGoalNumber()
{
	GoalNumber = UNBGenerateRandomNumberLibrary::GenerateRandomNumber();
}

void ANBGameState::OnRep_ChatMessages()
{
	if (ChatMessages.IsEmpty())
	{
		return;
	}

	static FString LastProcessedMessage = TEXT("");

	FString LastMessage = ChatMessages.Last();
	if (LastMessage != LastProcessedMessage)
	{
		UE_LOG(LogTemp, Warning, TEXT("Chat Updated! Last Message: %s"), *LastMessage);
		LastProcessedMessage = LastMessage;
	}
	MulticastChatMessage(LastMessage);
}

void ANBGameState::OnRep_GoalNumber()
{
	if (APlayerController* PlayerController = GetWorld()->GetFirstPlayerController())
	{
		if (ANBPlayerController* NBPlayerController = Cast<ANBPlayerController>(PlayerController))
		{
			if (UUserWidget* ChatWindowWidget = NBPlayerController->GetChatWindowWidget())
			{
				if (UTextBlock* GoalNumberText = Cast<UTextBlock>(ChatWindowWidget->GetWidgetFromName("GoalNumber")))
				{
					if (GoalNumberText)
					{
						GoalNumberText->SetText(FText::FromString(GoalNumber));

						UE_LOG(LogTemp, Warning, TEXT("OnRep_GoalNumber() Called!!"));
					}
				}
			}
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("OnRep_GoalNumber() called on: %s"), HasAuthority() ? TEXT("Server") : TEXT("Client"));
	UE_LOG(LogTemp, Warning, TEXT("Updated GoalNumber: %s"), *GoalNumber);
}

void ANBGameState::OnRep_bGameOver()
{
	UE_LOG(LogTemp, Warning, TEXT("OnRep_bGameOver called!!"));
}

void ANBGameState::OnRep_CurrentTurnPlayer()
{
	if (CurrentTurnPlayer == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("CurrentTurnPlayer is nullptr!"));
		return;  // nullptr인 경우 안전하게 종료
	}

	// 정상적인 경우에만 처리
	UE_LOG(LogTemp, Warning, TEXT("CurrentTurnPlayer updated: %s"), *CurrentTurnPlayer->GetName());
}

void ANBGameState::SwitchTurn()
{
	if (!CurrentTurnPlayer)
	{
		UE_LOG(LogTemp, Error, TEXT("No CurrentTurnPlayer set!"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("SwitchTurn Called! CurrentTurnPlayer: %s"), *CurrentTurnPlayer->GetName());

	APlayerController* NextTurnPlayer = nullptr;
	bool bFoundCurrentPlayer = false;

	// 모든 플레이어를 순회하며 턴을 전환
	for (TActorIterator<APlayerController> It(GetWorld()); It; ++It)
	{
		ANBPlayerController* NBPC = Cast<ANBPlayerController>(*It);
		if (NBPC)
		{
			// 현재 턴을 가진 플레이어를 찾음
			if (NBPC == CurrentTurnPlayer)
			{
				bFoundCurrentPlayer = true;
				// 현재 플레이어의 bIsMyTurn을 false로 설정
				ANBPlayerState* CurrentPlayerState = NBPC->GetPlayerState<ANBPlayerState>();
				if (CurrentPlayerState)
				{
					CurrentPlayerState->SetIsMyTurn(false);
				}
			}
			else if (!NextTurnPlayer)
			{
				// 다음 턴을 가진 플레이어를 찾음
				NextTurnPlayer = NBPC;
			}
		}
	}

	// 현재 턴을 가진 플레이어를 찾지 못한 경우
	if (!bFoundCurrentPlayer)
	{
		UE_LOG(LogTemp, Error, TEXT("CurrentTurnPlayer is not found in the game!"));
		return;
	}

	// 다음 턴을 가진 플레이어가 있으면, 턴을 전환
	if (NextTurnPlayer)
	{
		CurrentTurnPlayer = NextTurnPlayer;
		UE_LOG(LogTemp, Warning, TEXT("New Turn Player: %s"), *CurrentTurnPlayer->GetName());

		// 새로운 턴을 가진 플레이어의 bIsMyTurn을 true로 설정
		ANBPlayerState* NextPlayerState = CurrentTurnPlayer->GetPlayerState<ANBPlayerState>();
		if (NextPlayerState)
		{
			NextPlayerState->SetIsMyTurn(true);
		}
	}

	// UI 업데이트
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		ANBPlayerController* PlayerController = Cast<ANBPlayerController>(*It);
		if (PlayerController)
		{
			PlayerController->ClientUpdateChatWindowWidget();
		}
	}
}


void ANBGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ANBGameState, ChatMessages);
	DOREPLIFETIME(ANBGameState, GoalNumber);
	DOREPLIFETIME(ANBGameState, bGameOver);
	DOREPLIFETIME(ANBGameState, CurrentTurnPlayer);
}

void ANBGameState::MulticastChatMessage_Implementation(const FString& NewMessage)
{
	UE_LOG(LogTemp, Warning, TEXT("Broadcasting message: %s"), *NewMessage);
}