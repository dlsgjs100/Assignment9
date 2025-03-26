// Copyright Epic Games, Inc. All Rights Reserved.

#include "NBGameMode.h"
#include "NBPlayerController.h"
#include "NBGameState.h"
#include "NBGenerateRandomNumberLibrary.h"
#include "NBGameRulesLibrary.h"
#include "NBPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "EngineUtils.h"

ANBGameMode::ANBGameMode()
{
	Result = TEXT("");
	PlayerControllerClass = ANBPlayerController::StaticClass();
}

void ANBGameMode::SetupGoalNumber()
{
	ANBGameState* NBGameState = GetGameState<ANBGameState>();
	if (NBGameState && HasAuthority()) // ���������� ����
	{
		NBGameState->SetGoalNumber();
		NBGameState->ForceNetUpdate(); // ���� ������ ��� Ŭ���̾�Ʈ�� ����

		UE_LOG(LogTemp, Warning, TEXT("Server - GoalNumber set: %s"), *NBGameState->GoalNumber);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("SetupGoalNumber() was called but conditions were not met!"));
	}
}

void ANBGameMode::CompareMessagetoGoalNumber(const FString& Message, ANBPlayerController* NBPlayerController)
{
	ANBGameState* NBGameState = GetGameState<ANBGameState>();
	if (!NBGameState)
	{
		UE_LOG(LogTemp, Warning, TEXT("NBGameState is null!"));
		return;
	}
	if (NBGameState->GoalNumber.IsEmpty())
	{
		UE_LOG(LogTemp, Warning, TEXT("GoalNumber is not synced yet!"));
		return;
	}

	Result = UNBGameRulesLibrary::CheckStrikeBall(Message, NBGameState->GoalNumber);

	if (Result == TEXT("3S0B"))
	{
		UE_LOG(LogTemp, Warning, TEXT("Win!"));
		NBPlayerController->ServerWin();
	}
	else if (Result == TEXT("OUT"))
	{
		UE_LOG(LogTemp, Warning, TEXT("Defeat!"));
		NBPlayerController->ServerDefeat();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Continue!"));
		NBPlayerController->ServerContinue();
	}
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, Result);
}

void ANBGameMode::ClientSetUserID_Implementation(ANBPlayerController* NBPlayerController)
{
	if (NBPlayerController)
	{
		ANBPlayerState* NBPlayerState = NBPlayerController->GetPlayerState<ANBPlayerState>();
		if (NBPlayerController->IsLocalController())
		{
			NBPlayerState->SetUserID("Host");
		}
		else
		{
			NBPlayerState->SetUserID("Guest");
		}
	}
}

void ANBGameMode::ServerSetTurnOrder_Implementation()
{
	TArray<ANBPlayerController*> PlayerControllers;

	// ���� ������ ��� �÷��̾� ��Ʈ�ѷ� ��������
	for (TActorIterator<APlayerController> It(GetWorld()); It; ++It)
	{
		ANBPlayerController* NBPC = Cast<ANBPlayerController>(*It);
		if (NBPC)
		{
			PlayerControllers.Add(NBPC);
		}
	}

	// �÷��̾ 2�� �̻��� ���� ����
	if (PlayerControllers.Num() < 2)
	{
		UE_LOG(LogTemp, Error, TEXT("Not enough players to assign turn order! Waiting..."));

		// ���� �ð� �� �ٽ� üũ
		GetWorld()->GetTimerManager().SetTimer(
			TurnOrderRetryTimer,
			this,
			&ANBGameMode::ServerSetTurnOrder,
			1.0f,
			false);
		return;
	}

	// �������� ù ��° �� �÷��̾� ����
	int FirstPlayerIndex = FMath::RandRange(0, 1);
	APlayerController* FirstPlayer = nullptr;

	for (int i = 0; i < PlayerControllers.Num(); i++)
	{
		ANBPlayerController* NBPC = PlayerControllers[i];
		if (NBPC)
		{
			ANBPlayerState* NBPS = NBPC->GetPlayerState<ANBPlayerState>();
			if (NBPS)
			{
				bool bIsFirst = (i == FirstPlayerIndex);
				NBPS->SetIsMyTurn(bIsFirst);

				if (bIsFirst)
				{
					FirstPlayer = NBPC;
				}

				UE_LOG(LogTemp, Warning, TEXT("%s is %s"),
					*NBPC->GetName(),
					bIsFirst ? TEXT("first") : TEXT("second"));

				// Ŭ���̾�Ʈ���� ����
				NBPC->ClientReceiveTurnOrder(bIsFirst);
			}
		}
	}

	// **GameState�� CurrentTurnPlayer ����**
	ANBGameState* GS = GetGameState<ANBGameState>();
	if (GS && FirstPlayer)
	{
		GS->CurrentTurnPlayer = FirstPlayer;
		UE_LOG(LogTemp, Warning, TEXT("CurrentTurnPlayer set to: %s"), *FirstPlayer->GetName());
	}
}


