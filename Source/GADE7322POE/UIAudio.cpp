// Copyright Epic Games, Inc. All Rights Reserved.

#include "UIAudio.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SlateSound.h"
#include "Sound/SoundBase.h"
#include "Styling/SlateTypes.h"

USoundBase* UIAudio::GetButtonPressSound()
{
	static TWeakObjectPtr<USoundBase> CachedSound;
	if (USoundBase* Sound = CachedSound.Get())
	{
		return Sound;
	}

	USoundBase* Sound = LoadObject<USoundBase>(nullptr, TEXT("/Game/UltimateUIMenusSFX/Fantasy_RPG/Cue/FantasyRPG_03_Cue.FantasyRPG_03_Cue"));
	if (!Sound)
	{
		Sound = LoadObject<USoundBase>(nullptr, TEXT("/Game/UltimateUIMenusSFX/Fantasy_RPG/Wav/FantasyRPG_03.FantasyRPG_03"));
	}

	CachedSound = Sound;
	return Sound;
}

void UIAudio::PlayButtonPressSound(const UObject* WorldContext)
{
	if (USoundBase* Sound = GetButtonPressSound())
	{
		UGameplayStatics::PlaySound2D(WorldContext, Sound);
	}
}

void UIAudio::BindButtonPressSound(UButton* Button)
{
	if (!Button)
	{
		return;
	}

	USoundBase* Sound = GetButtonPressSound();
	if (!Sound)
	{
		return;
	}

	FSlateSound SlateSound;
	SlateSound.SetResourceObject(Sound);

	FButtonStyle Style = Button->GetStyle();
	Style.SetPressedSound(SlateSound);
	Button->SetStyle(Style);
}
