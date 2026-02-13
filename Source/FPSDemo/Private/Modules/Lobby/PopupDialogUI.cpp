#include "Modules/Lobby/PopupDialogUI.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

void UPopupDialogUI::NativeConstruct()
{
    Super::NativeConstruct();

    Button_Ok->OnClicked.AddDynamic(this, &UPopupDialogUI::HandleOkClicked);
    Button_Cancel->OnClicked.AddDynamic(this, &UPopupDialogUI::HandleCancelClicked);
}

void UPopupDialogUI::Setup(
    const FString& InMessage,
    TFunction<void()> InOnOk,
    TFunction<void()> InOnCancel
)
{
	Button_Cancel->SetVisibility(InOnCancel ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
    TextBlock_Message->SetText(FText::FromString(InMessage));
    OnOk = MoveTemp(InOnOk);
    OnCancel = MoveTemp(InOnCancel);
}

void UPopupDialogUI::HandleOkClicked()
{
    if (OnOk)
    {
        OnOk();
    }

    RemoveFromParent();
}

void UPopupDialogUI::HandleCancelClicked()
{
    if (OnCancel)
    {
        OnCancel();
    }

    RemoveFromParent();
}
