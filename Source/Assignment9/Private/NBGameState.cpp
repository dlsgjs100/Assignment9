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
	OnRep_GoalNumber();
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
					}
				}
			}
		}
	}
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
		return;  // nullptr�� ��� �����ϰ� ����
	}

	// �������� ��쿡�� ó��
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

	// ��� �÷��̾ ��ȸ�ϸ� ���� ��ȯ
	for (TActorIterator<APlayerController> It(GetWorld()); It; ++It)
	{
		ANBPlayerController* NBPC = Cast<ANBPlayerController>(*It);
		if (NBPC)
		{
			// ���� ���� ���� �÷��̾ ã��
			if (NBPC == CurrentTurnPlayer)
			{
				bFoundCurrentPlayer = true;
				// ���� �÷��̾��� bIsMyTurn�� false�� ����
				ANBPlayerState* CurrentPlayerState = NBPC->GetPlayerState<ANBPlayerState>();
				if (CurrentPlayerState)
				{
					CurrentPlayerState->SetIsMyTurn(false);
				}
			}
			else if (!NextTurnPlayer)
			{
				// ���� ���� ���� �÷��̾ ã��
				NextTurnPlayer = NBPC;
			}
		}
	}

	// ���� ���� ���� �÷��̾ ã�� ���� ���
	if (!bFoundCurrentPlayer)
	{
		UE_LOG(LogTemp, Error, TEXT("CurrentTurnPlayer is not found in the game!"));
		return;
	}

	// ���� ���� ���� �÷��̾ ������, ���� ��ȯ
	if (NextTurnPlayer)
	{
		CurrentTurnPlayer = NextTurnPlayer;
		UE_LOG(LogTemp, Warning, TEXT("New Turn Player: %s"), *CurrentTurnPlayer->GetName());

		// ���ο� ���� ���� �÷��̾��� bIsMyTurn�� true�� ����
		ANBPlayerState* NextPlayerState = CurrentTurnPlayer->GetPlayerState<ANBPlayerState>();
		if (NextPlayerState)
		{
			NextPlayerState->SetIsMyTurn(true);
		}
	}

	// UI ������Ʈ
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
	DOREPLIFETIME(ANBGameState, GoalNumber);
	DOREPLIFETIME(ANBGameState, bGameOver);
	DOREPLIFETIME(ANBGameState, CurrentTurnPlayer);
}

void ANBGameState::MulticastShowGameResult_Implementation(const FString& UserID)
{
	if (HasAuthority())
	{
		return;
	}

	FString WinMessage = FString::Printf(TEXT("%s Won!!"), *UserID);
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, WinMessage);
}