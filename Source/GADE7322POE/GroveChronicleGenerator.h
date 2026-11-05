// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "TowerDefenseTypes.h"

namespace GroveChronicleGenerator
{
	FGroveChronicle Generate(int32 Seed);
	FText GetWaveFlavour(const FGroveChronicle& Chronicle, int32 WaveNumber, int32 UpgradePower);
}
