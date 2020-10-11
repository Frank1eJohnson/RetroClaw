#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "ClawGameMode.h"
#include "ClawGameHUD.generated.h"


/**
 *
 */
UCLASS()
class RETROCLAW_API UClawGameHUD : public UUserWidget
{
    GENERATED_BODY()

protected:
    UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
    class UTextBlock* ScoreCount;

    UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
    class UTextBlock* HealthPane;
public:

    UFUNCTION(BlueprintCallable)
        void SetHealthCount(int32 Count);

    UFUNCTION(BlueprintCallable)
        void InitializeHUD(AClawGameMode* RunGameMode);
    UFUNCTION(BlueprintCallable)
        void SetScoreCount(int32 Count);
};
