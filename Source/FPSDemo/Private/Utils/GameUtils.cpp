// Fill out your copyright notice in the Description page of Project Settings.


#include "Utils/GameUtils.h"

GameUtils::GameUtils()
{
}

GameUtils::~GameUtils()
{
}


FString GameUtils::PointNumber(int32 Number)
{
    FString NumStr = FString::FromInt(Number);
    int32 Len = NumStr.Len();

    FString Result;
    Result.Reserve(Len + (Len / 3));

    int32 Count = 0;

    for (int32 i = Len - 1; i >= 0; i--)
    {
        Result.InsertAt(0, NumStr[i]);
        Count++;

        if (Count == 3 && i != 0)
        {
            Result.InsertAt(0, TEXT("."));
            Count = 0;
        }
    }

    return Result;
}

FString GameUtils::GenerateMd5Token()
{
    const FString Raw =
        FGuid::NewGuid().ToString(EGuidFormats::Digits) +
        FPlatformMisc::GetLoginId();
    return FMD5::HashAnsiString(*Raw);
}

UTexture2D* GameUtils::GetTextureAvatar(const FString& AvatarId)
{
    const FString Path = FString::Printf(
        TEXT("/Game/Main/Asset/Avatars/avatar_%s.avatar_%s"),
        *AvatarId,
        *AvatarId
    );

    return LoadObject<UTexture2D>(nullptr, *Path);
}