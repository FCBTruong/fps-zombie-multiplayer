// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/UI/Crosshair.h"
#include <Components/CanvasPanelSlot.h>

FUECrosshairData UCrosshair::ParseCrosshairCode(const FString& Input)
{
    // 1) Zero-init the struct
    FUECrosshairData OutData{};

    // 2) Set your desired defaults IN CPP here
    OutData.CrosshairColor = FLinearColor::White;
    OutData.OutlineEnabled = false;
    OutData.OutlineOpacity = 1.0f;

    // Inner defaults
    OutData.InnerThickness = 2.0f;   // important: > 0 or line is invisible
    OutData.InnerLength = 6.0f;
    OutData.InnerOffset = 3.0f;
    OutData.InnerAlpha = 1.0f;
    OutData.InnerFiringError = false;

    // Outer defaults (adjust as you like)
    OutData.OuterThickness = 0.0f;
    OutData.OuterLength = 0.0f;
    OutData.OuterOffset = 0.0f;
    OutData.OuterAlpha = 0.0f;
    OutData.OuterFiringError = false;

    OutData.DotEnable = false;

    // 3) Then parse and override only keys that are present
    TArray<FString> Tokens;
    Input.ParseIntoArray(Tokens, TEXT(";"), true);

    TSet<FString> ValidKeys = {
		"c","h","m", "u",
        "0t","0l","0o","0a","0f",
        "1t","1l","1o","1a","1f",
        "1b"
    };

    auto ToFloat = [&](const FString& S) { return FCString::Atof(*S); };
    auto ToBool = [&](const FString& S) { return S == "1"; };

    for (int32 i = 0; i < Tokens.Num(); i++)
    {
        const FString& Key = Tokens[i];

        if (!ValidKeys.Contains(Key))
            continue;

        if (i + 1 >= Tokens.Num())
        {
            UE_LOG(LogTemp, Warning, TEXT("Key %s has no value"), *Key);
            break;
        }

        const FString& Val = Tokens[i + 1];

        if (Key == "c")
        {
            int32 idx = FCString::Atoi(*Val);
            switch (idx)
            {
            case 0: // White
                OutData.CrosshairColor = FLinearColor::White;
                break;

            case 1: // Green (#00FF00)
                OutData.CrosshairColor = FLinearColor(0.f, 1.f, 0.f, 1.f);
                break;

            case 2: // Yellow Green (#7FFF00)
                OutData.CrosshairColor = FLinearColor(127.f / 255.f, 1.f, 0.f, 1.f);
                break;

            case 3: // Green Yellow (#DFFF00)
                OutData.CrosshairColor = FLinearColor(223.f / 255.f, 1.f, 0.f, 1.f);
                break;

            case 4: // Yellow (#FFFF00)
                OutData.CrosshairColor = FLinearColor(1.f, 1.f, 0.f, 1.f);
                break;

            case 5: // Cyan (#00FFFF)
                OutData.CrosshairColor = FLinearColor(0.f, 1.f, 1.f, 1.f);
                break;

            case 6: // Pink (#FF00FF)
                OutData.CrosshairColor = FLinearColor(1.f, 0.f, 1.f, 1.f);
                break;

            case 7: // Red (#FF0000)
                OutData.CrosshairColor = FLinearColor(1.f, 0.f, 0.f, 1.f);
                break;

            case 8:
                // Custom color – will be overridden by the 'u;RRGGBBAA' token
                // You can either leave current color or set a temporary default here.
                break;

            default:
                OutData.CrosshairColor = FLinearColor::White;
                break;
            }

        }
        else if (Key == "h")  OutData.OutlineEnabled = ToBool(Val);
        else if (Key == "m")  OutData.OutlineOpacity = ToFloat(Val);

        else if (Key == "0t") OutData.InnerThickness = ToFloat(Val);
        else if (Key == "0l") OutData.InnerLength = ToFloat(Val);
        else if (Key == "0o") OutData.InnerOffset = ToFloat(Val);
        else if (Key == "0a") OutData.InnerAlpha = ToFloat(Val);
        else if (Key == "0f") OutData.InnerFiringError = ToBool(Val);

        else if (Key == "1t") OutData.OuterThickness = ToFloat(Val);
        else if (Key == "1l") OutData.OuterLength = ToFloat(Val);
        else if (Key == "1o") OutData.OuterOffset = ToFloat(Val);
        else if (Key == "1a") OutData.OuterAlpha = ToFloat(Val);
        else if (Key == "1f") OutData.OuterFiringError = ToBool(Val);
        else if (Key == "u")
        {
            // Expect hex string: RRGGBBAA
            if (Val.Len() == 8)
            {
                auto HexToByte = [&](TCHAR C1, TCHAR C2)
                    {
                        FString Pair;
                        Pair.AppendChar(C1);
                        Pair.AppendChar(C2);
                        return FParse::HexDigit(Pair[0]) * 16 + FParse::HexDigit(Pair[1]);
                    };

                uint8 R = HexToByte(Val[0], Val[1]);
                uint8 G = HexToByte(Val[2], Val[3]);
                uint8 B = HexToByte(Val[4], Val[5]);
                uint8 A = HexToByte(Val[6], Val[7]);

                OutData.CrosshairColor = FLinearColor(
                    R / 255.f,
                    G / 255.f,
                    B / 255.f,
                    A / 255.f);

                UE_LOG(LogTemp, Warning, TEXT("Parsed custom color: %s"), *OutData.CrosshairColor.ToString());
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("Invalid u; hex length (expected 8): %s"), *Val);
            }
        }


        else if (Key == "1b") OutData.DotEnable = ToBool(Val);

        i++; // skip value
    }

    return OutData;
}

void UCrosshair::ApplyData(const FUECrosshairData& D)
{
    UE_LOG(LogTemp, Warning, TEXT("ApplyData crosshair"));

    auto SetLine =
        [&](UImage* Line, float Length, float Thickness, float Offset, FVector2D Dir, float Alpha, const FLinearColor& Color)
        {
            if (!Line)
                return;

            // Determine if vertical
            bool bVertical = FMath::Abs(Dir.Y) > 0.5f;

            FVector2D Size = bVertical
                ? FVector2D(Thickness, Length)   // vertical
                : FVector2D(Length, Thickness);  // horizontal

            UE_LOG(LogTemp, Warning, TEXT("SetLine Size: %s"), *Size.ToString());

            if (auto* Slot = Cast<UCanvasPanelSlot>(Line->Slot))
            {
                Slot->SetAutoSize(false);
                Slot->SetSize(Size);
            }

            // Position relative to center
            FVector2D Pos = Dir * (Offset + Length * 0.5f);
            Line->SetRenderTranslation(Pos);

            // Apply color + alpha
            FLinearColor FinalColor = Color;
            FinalColor.A = Alpha;

            UE_LOG(LogTemp, Warning, TEXT("SetLine Color: %s"), *FinalColor.ToString());

            Line->SetColorAndOpacity(FinalColor);
        };

    //
    // INNER LINES
    //
    SetLine(InnerTop, D.InnerLength, D.InnerThickness, D.InnerOffset, FVector2D(0, -1), D.InnerAlpha, D.CrosshairColor);
    SetLine(InnerBottom, D.InnerLength, D.InnerThickness, D.InnerOffset, FVector2D(0, 1), D.InnerAlpha, D.CrosshairColor);
    SetLine(InnerLeft, D.InnerLength, D.InnerThickness, D.InnerOffset, FVector2D(-1, 0), D.InnerAlpha, D.CrosshairColor);
    SetLine(InnerRight, D.InnerLength, D.InnerThickness, D.InnerOffset, FVector2D(1, 0), D.InnerAlpha, D.CrosshairColor);

    //
    // OUTER LINES
    //
    SetLine(OuterTop, D.OuterLength, D.OuterThickness, D.OuterOffset, FVector2D(0, -1), D.OuterAlpha, D.CrosshairColor);
    SetLine(OuterBottom, D.OuterLength, D.OuterThickness, D.OuterOffset, FVector2D(0, 1), D.OuterAlpha, D.CrosshairColor);
    SetLine(OuterLeft, D.OuterLength, D.OuterThickness, D.OuterOffset, FVector2D(-1, 0), D.OuterAlpha, D.CrosshairColor);
    SetLine(OuterRight, D.OuterLength, D.OuterThickness, D.OuterOffset, FVector2D(1, 0), D.OuterAlpha, D.CrosshairColor);

    //
    // DOT
    //
    if (Dot)
    {
        Dot->SetVisibility(D.DotEnable ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
        Dot->SetColorAndOpacity(D.CrosshairColor);
    }
}

void UCrosshair::NativeConstruct()
{
    Super::NativeConstruct();
}

void UCrosshair::SetCrosshairCode(const FString& NewCode)
{
    if (NewCode.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("SetCrosshairCode: Empty code, using default"));
        return SetCrosshairCode(CrosshairCodeDefault);
	}
    FUECrosshairData Data;
    Data = ParseCrosshairCode(NewCode);
    ApplyData(Data);
}