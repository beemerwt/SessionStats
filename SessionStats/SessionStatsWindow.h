#pragma once
#include "bakkesmod/plugin/bakkesmodplugin.h"
#include "bakkesmod/plugin/pluginwindow.h"
#include "Constants.h"

class SessionStatsPlugin;
struct ImGuiStyle;

class SessionStatsWindow {
private:
	SessionStatsPlugin* parent;
	bool isWindowOpen = false;
	bool block_input = false;

	// Canvas Render boolean
	bool displayOnMenu = false;
	int currentPlaylist = 0;

public:
	SessionStatsWindow(SessionStatsPlugin* parent);

	std::string GetMenuName();
	std::string GetMenuTitle();

	bool ShouldDisplayOnMenu();
	bool ShouldBlockInput();
	bool IsActiveOverlay();

	void SetImGuiContext(uintptr_t ctx);

	void OnOpen();
	void OnClose();
	void Render();
	void DisplaySessionStats(ImGuiStyle& style);
	void DisplaySessionSettings(ImGuiStyle& style);
};

