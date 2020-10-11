// Fill out your copyright notice in the Description page of Project Settings.


#include "ClawGameHUD.h"
#include "ClawGameMode.h"

void UClawGameHUD::InitializeHUD(AClawGameMode* RunGameMode)
{
	if (RunGameMode)
	{
		ScoreCount->SetText(FText::AsNumber(0));
		RunGameMode->OnScoreCountChanged.AddDynamic(this, &UClawGameHUD::SetScoreCount);

		HealthPane->SetText(FText::AsNumber(100));
		RunGameMode->OnHealthPaneChanged.AddDynamic(this, &UClawGameHUD::SetHealthCount);
	}
}

void UClawGameHUD::SetScoreCount(int32 Count)
{
	ScoreCount->SetText(FText::AsNumber(Count));
}
void UClawGameHUD::SetHealthCount(const int32 Count)
{
	HealthPane->SetText(FText::AsNumber(Count));
}

