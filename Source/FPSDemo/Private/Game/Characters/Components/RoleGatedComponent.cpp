// Fill out your copyright notice in the Description page of Project Settings.


#include "Game/Characters/Components/RoleGatedComponent.h"

URoleGatedComponent::URoleGatedComponent()
{

}

void URoleGatedComponent::SetEnabled(bool bInEnabled)
{
    if (bEnabled == bInEnabled) return;
    bEnabled = bInEnabled;

    if (bEnabled) Activate(true);
    else          Deactivate();

    OnEnabledChanged(bEnabled);
}

