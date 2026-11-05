// Copyright Epic Games, Inc. All Rights Reserved.

#include "GroveChronicleGenerator.h"

namespace
{
const TCHAR* GroveNames[] = {
	TEXT("Whispering Canopy"),
	TEXT("Howler Hollow"),
	TEXT("Golden Liana"),
	TEXT("Storm-Root Basin"),
	TEXT("Monkey-King Clearing"),
	TEXT("Vinefall Ridge"),
	TEXT("Sapfire Glade"),
	TEXT("Drum-Root Terrace")
};

const TCHAR* Omens[] = {
	TEXT("The drums start before sunrise."),
	TEXT("Gold sap weeps from the heartwood."),
	TEXT("A silent nest watches the path."),
	TEXT("Lianas tighten when the wind stills."),
	TEXT("Banana flowers open the wrong way."),
	TEXT("The river smells of iron and fruit.")
};

const TCHAR* BriefOpeners[] = {
	TEXT("Scouts marked this grove as restless. Hold the heartwood while the paths rewrite themselves."),
	TEXT("The canopy remembers every raid. Spend coin on roots or steel — both will be tested."),
	TEXT("A new seed means a new map and a new omen. Place defenders before the first howl."),
	TEXT("The grove will not keep last game's story. Read the omen, then choose upgrades or numbers.")
};

const TCHAR* WaveLines[] = {
	TEXT("Wave {0}: rustle on the outer roots."),
	TEXT("Wave {0}: howlers test the eastern vine."),
	TEXT("Wave {0}: chimps stitch the treeline."),
	TEXT("Wave {0}: gold-sap drums answer the tower."),
	TEXT("Wave {0}: the canopy leans toward the heartwood."),
	TEXT("Wave {0}: a heavier tread follows the path.")
};

FText PickText(FRandomStream& Stream, const TCHAR* const* Table, int32 Count)
{
	const int32 Index = Stream.RandRange(0, Count - 1);
	return FText::FromString(Table[Index]);
}
}

FGroveChronicle GroveChronicleGenerator::Generate(int32 Seed)
{
	FRandomStream Stream;
	Stream.Initialize(Seed == 0 ? 1 : Seed);

	FGroveChronicle Chronicle;
	Chronicle.Seed = Seed;
	Chronicle.GroveName = PickText(Stream, GroveNames, UE_ARRAY_COUNT(GroveNames));
	Chronicle.Omen = PickText(Stream, Omens, UE_ARRAY_COUNT(Omens));
	Chronicle.OpeningBrief = FText::Format(
		NSLOCTEXT("TowerDefense", "GroveBrief", "{0}  Omen: {1}  {2}"),
		Chronicle.GroveName,
		Chronicle.Omen,
		PickText(Stream, BriefOpeners, UE_ARRAY_COUNT(BriefOpeners)));

	for (int32 Index = 0; Index < UE_ARRAY_COUNT(WaveLines); ++Index)
	{
		Chronicle.WaveProphecies.Add(FText::FromString(WaveLines[Index]));
	}

	return Chronicle;
}

FText GroveChronicleGenerator::GetWaveFlavour(const FGroveChronicle& Chronicle, int32 WaveNumber, int32 UpgradePower)
{
	if (WaveNumber <= 0)
	{
		return Chronicle.OpeningBrief;
	}

	FText Line = Chronicle.OpeningBrief;
	if (Chronicle.WaveProphecies.Num() > 0)
	{
		const int32 Index = FMath::Abs(Chronicle.Seed + WaveNumber) % Chronicle.WaveProphecies.Num();
		Line = FText::Format(Chronicle.WaveProphecies[Index], FText::AsNumber(WaveNumber));
	}

	if (UpgradePower >= 4)
	{
		return FText::Format(
			NSLOCTEXT("TowerDefense", "GroveWaveUpgraded", "{0} The grove answers steel with heavier tread."),
			Line);
	}

	if (WaveNumber >= 5)
	{
		return FText::Format(
			NSLOCTEXT("TowerDefense", "GroveWaveLate", "{0} The omen thickens."),
			Line);
	}

	return Line;
}
