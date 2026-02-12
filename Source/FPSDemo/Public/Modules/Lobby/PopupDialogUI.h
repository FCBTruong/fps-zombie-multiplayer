#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PopupDialogUI.generated.h"

UCLASS()
class UPopupDialogUI : public UUserWidget
{
    GENERATED_BODY()

public:
    void Setup(
        const FString& InMessage,
        TFunction<void()> InOnOk,
        TFunction<void()> InOnCancel
    );

protected:
    virtual void NativeConstruct() override;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* TextBlock_Message;

    UPROPERTY(meta = (BindWidget))
    class UButton* Button_Ok;

    UPROPERTY(meta = (BindWidget))
    class UButton* Button_Cancel;

private:
    TFunction<void()> OnOk;
    TFunction<void()> OnCancel;

    UFUNCTION()
    void HandleOkClicked();

    UFUNCTION()
    void HandleCancelClicked();
};
