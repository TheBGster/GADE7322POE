// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class UButton;
class UObject;
class USoundBase;

namespace UIAudio
{
	USoundBase* GetButtonPressSound();
	void PlayButtonPressSound(const UObject* WorldContext);
	void BindButtonPressSound(UButton* Button);
}
