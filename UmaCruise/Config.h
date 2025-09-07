#pragma once
#include "boost/filesystem.hpp"

struct Config
{
	int		refreshInterval = 1;
	bool	autoStart = false;
	bool	stopUpdatePreviewOnTraining = false;
	bool	popupRaceListWindow = false;
	bool	notifyFavoriteRaceHold = true;
	enum Theme {
		kAuto, kDark, kLight,
	};
	Theme	theme = kAuto;
	bool	windowTopMost = false;
	boost::filesystem::path screenShotFolder;

	enum ScreenCaptureMethod {
		kGDI, kDesktopDuplication, kWindowsGraphicsCapture,
	};
	ScreenCaptureMethod	screenCaptureMethod = kGDI;

	// Game text/data locale for OCR and event library
	enum DataLocale {
		kJP = 0,
		kEN = 1,
	};
	DataLocale dataLocale = kEN; // default to EN for this fork

	bool	LoadConfig();
	void	SaveConfig();
};
