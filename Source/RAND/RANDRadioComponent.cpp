// Copyright Fortitudo Studio. All Rights Reserved.

#include "RANDRadioComponent.h"
#include "TimeComponent.h"

URANDRadioComponent::URANDRadioComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void URANDRadioComponent::BeginPlay()
{
	Super::BeginPlay();

	BuildPlaylists();

	if (URANDTimeComponent* Clock = URANDTimeComponent::Get(this))
	{
		Clock->OnMinutePassed.AddDynamic(this, &URANDRadioComponent::HandleMinutePassed);
	}
}

void URANDRadioComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (URANDTimeComponent* Clock = URANDTimeComponent::Get(this))
	{
		Clock->OnMinutePassed.RemoveDynamic(this, &URANDRadioComponent::HandleMinutePassed);
	}
	Super::EndPlay(EndPlayReason);
}

void URANDRadioComponent::BuildPlaylists()
{
	Playlists.SetNum(StationCount);

	auto Track = [](const TCHAR* StationName, const TCHAR* TrackName, const TCHAR* Artist, int32 Duration)
	{
		FRadioTrack T;
		T.StationName = StationName;
		T.TrackName = TrackName;
		T.Artist = Artist;
		T.Duration = Duration;
		return T;
	};

	// Placeholder rotations -- style references for the eventual licensed/original
	// tracks. Names are indicative of each station's format, not real recordings.
	Playlists[static_cast<int32>(ERadioStation::Station947)] = {
		Track(TEXT("947"), TEXT("Highveld Morning"), TEXT("The Jozi Collective"), 3),
		Track(TEXT("947"), TEXT("Sandton Skyline"), TEXT("Nadia Blue"), 4),
		Track(TEXT("947"), TEXT("Golden Hour"), TEXT("Cassidy Moore"), 3),
		Track(TEXT("947"), TEXT("Weekend Traffic"), TEXT("Neon Republic"), 4),
		Track(TEXT("947"), TEXT("Summer Rain"), TEXT("Lelo & The Lights"), 3),
	};

	Playlists[static_cast<int32>(ERadioStation::YFM)] = {
		Track(TEXT("YFM"), TEXT("Sponono Yami"), TEXT("DJ Mkhathini"), 5),
		Track(TEXT("YFM"), TEXT("Log Drum Season"), TEXT("Kabelo Deep"), 6),
		Track(TEXT("YFM"), TEXT("Braam Nights"), TEXT("Tumi Wave"), 4),
		Track(TEXT("YFM"), TEXT("Ziyakhala"), TEXT("MaWhoo Zn"), 5),
		Track(TEXT("YFM"), TEXT("Yanos for the Road"), TEXT("Papa Ceez"), 6),
	};

	Playlists[static_cast<int32>(ERadioStation::RSG)] = {
		Track(TEXT("RSG"), TEXT("Bokkie van die Bosveld"), TEXT("Hannes Kruger"), 3),
		Track(TEXT("RSG"), TEXT("Highveld se Wind"), TEXT("Marlene du Toit"), 4),
		Track(TEXT("RSG"), TEXT("Pad na Pretoria"), TEXT("Riaan Steyn"), 3),
		Track(TEXT("RSG"), TEXT("Lekker Sondagmiddag"), TEXT("Die Kaalvoet Broers"), 4),
		Track(TEXT("RSG"), TEXT("Stil Water"), TEXT("Anneke Vorster"), 3),
	};

	Playlists[static_cast<int32>(ERadioStation::TalkRadio)] = {
		Track(TEXT("Talk Radio"), TEXT("The Tender Files"), TEXT("Panel Discussion"), 8),
		Track(TEXT("Talk Radio"), TEXT("Load Shedding Phone-In"), TEXT("Listener Calls"), 6),
		Track(TEXT("Talk Radio"), TEXT("Municipal Watch"), TEXT("Investigative Desk"), 7),
		Track(TEXT("Talk Radio"), TEXT("Business Hour"), TEXT("Markets Update"), 5),
		Track(TEXT("Talk Radio"), TEXT("Crime Line"), TEXT("Community Report"), 6),
	};
}

const FRadioTrack& URANDRadioComponent::GetCurrentTrack() const
{
	static const FRadioTrack Empty;

	const int32 Idx = static_cast<int32>(Station);
	if (!Playlists.IsValidIndex(Idx) || Playlists[Idx].Num() == 0)
	{
		return Empty;
	}
	const int32 Track = TrackIndex[Idx] % Playlists[Idx].Num();
	return Playlists[Idx][Track];
}

FString URANDRadioComponent::StationToString(ERadioStation InStation)
{
	switch (InStation)
	{
	case ERadioStation::YFM:       return TEXT("YFM");
	case ERadioStation::RSG:       return TEXT("RSG");
	case ERadioStation::TalkRadio: return TEXT("Talk Radio");
	case ERadioStation::Station947:
	default:                       return TEXT("947");
	}
}

FString URANDRadioComponent::GetDisplayString() const
{
	const FRadioTrack& Track = GetCurrentTrack();
	if (Track.TrackName.IsEmpty())
	{
		return StationToString(Station);
	}
	return FString::Printf(TEXT("%s  %s / %s"),
		*StationToString(Station), *Track.TrackName, *Track.Artist);
}

void URANDRadioComponent::SetStation(ERadioStation NewStation)
{
	if (NewStation == Station)
	{
		return;
	}

	Station = NewStation;
	MinutesOnTrack = 0;

	OnStationChanged.Broadcast(Station);
	OnTrackChanged.Broadcast(GetCurrentTrack());
}

void URANDRadioComponent::CycleStation()
{
	const int32 Next = (static_cast<int32>(Station) + 1) % StationCount;
	SetStation(static_cast<ERadioStation>(Next));
}

void URANDRadioComponent::NextTrack()
{
	const int32 Idx = static_cast<int32>(Station);
	if (!Playlists.IsValidIndex(Idx) || Playlists[Idx].Num() == 0)
	{
		return;
	}

	TrackIndex[Idx] = (TrackIndex[Idx] + 1) % Playlists[Idx].Num();
	MinutesOnTrack = 0;
	OnTrackChanged.Broadcast(GetCurrentTrack());
}

void URANDRadioComponent::SetPlaying(bool bNewPlaying)
{
	if (bPlaying == bNewPlaying)
	{
		return;
	}

	bPlaying = bNewPlaying;
	if (bPlaying)
	{
		// Announce what's on as André gets in.
		OnStationChanged.Broadcast(Station);
		OnTrackChanged.Broadcast(GetCurrentTrack());
	}
}

void URANDRadioComponent::HandleMinutePassed(int32 /*Day*/, int32 /*Hour*/, int32 /*Minute*/)
{
	if (!bPlaying)
	{
		return; // Only burn through the playlist while someone is listening.
	}

	const FRadioTrack& Track = GetCurrentTrack();
	if (Track.Duration <= 0)
	{
		return;
	}

	if (++MinutesOnTrack >= Track.Duration)
	{
		NextTrack();
	}
}
