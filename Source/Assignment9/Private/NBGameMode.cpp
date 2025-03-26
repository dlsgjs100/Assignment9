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
	if (NBGameState && HasAuthority()) // 서버에서만 실행
	{
		NBGameState->SetGoalNumber();
		NBGameState->ForceNetUpdate(); // 변경 사항을 즉시 클라이언트로 전파

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

	// 현재 접속한 모든 플레이어 컨트롤러 가져오기
	for (TActorIterator<APlayerController> It(GetWorld()); It; ++It)
	{
		ANBPlayerController* NBPC = Cast<ANBPlayerController>(*It);
		if (NBPC)
		{
			PlayerControllers.Add(NBPC);
		}
	}

	// 플레이어가 2명 이상일 때만 진행
	if (PlayerControllers.Num() < 2)
	{
		UE_LOG(LogTemp, Error, TEXT("Not enough players to assign turn order! Waiting..."));

		// 일정 시간 후 다시 체크
		GetWorld()->GetTimerManager().SetTimer(
			TurnOrderRetryTimer,
			this,
			&ANBGameMode::ServerSetTurnOrder,
			1.0f,
			false);
		return;
	}

	// 랜덤으로 첫 번째 턴 플레이어 선정
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

				// 클라이언트에게 전파
				NBPC->ClientReceiveTurnOrder(bIsFirst);
			}
		}
	}

	// **GameState에 CurrentTurnPlayer 설정**
	ANBGameState* GS = GetGameState<ANBGameState>();
	if (GS && FirstPlayer)
	{
		GS->CurrentTurnPlayer = FirstPlayer;
		UE_LOG(LogTemp, Warning, TEXT("CurrentTurnPlayer set to: %s"), *FirstPlayer->GetName());
	}
}


