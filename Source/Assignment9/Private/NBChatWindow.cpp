// Copyright Epic Games, Inc. All Rights Reserved.

#include "NBChatWindow.h"
#include "NBPlayerController.h"
#include "Components/EditableTextBox.h"

void UNBChatWindow::NativeConstruct()
{
	Super::NativeConstruct();

	APlayerController* PlayerController = GetOwningPlayer();

	if (UserInputTextBox)
	{
		UserInputTextBox->OnTextCommitted.AddDynamic(this, &UNBChatWindow::OnUserInputTextCommitted);
	}
}

void UNBChatWindow::OnUserInputTextCommitted(const FText& Text, ETextCommit::Type CommitType)
{
    UE_LOG(LogTemp, Warning, TEXT("OnUserInputTextCommitted called"));

    if (CommitType == ETextCommit::OnEnter)
    {
        UE_LOG(LogTemp, Warning, TEXT("User Input Committed : %s"), *Text.ToString());

        if (APlayerController* PlayerController = GetOwningPlayer())
        {
            ANBPlayerController* NBPlayerController = Cast<ANBPlayerController>(PlayerController);
            if (NBPlayerController)
            {
                    NBPlayerController->ServerSendChatMessage(Text.ToString());
            }
        }

        if (UserInputTextBox)
        {
            UserInputTextBox->SetText(FText::GetEmpty());
        }
    }
}

