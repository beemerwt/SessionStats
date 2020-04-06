#pragma once
/*
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#pragma comment( lib, "bakkesmod.lib" )
#include "SessionStatsWindow.h"
#include <map> 
#include <string>

typedef struct {
	float initialMMR, currentMMR, lastMMR;
	int wins, losses, streak;
	int tier, div;
} StatsStruct;

class SessionStatsPlugin : public BakkesMod::Plugin::BakkesModPlugin, public BakkesMod::Plugin::PluginWindow
{
private:
	SessionStatsWindow* pluginWindow;
		
	std::map<int, StatsStruct> stats;
	int currentPlaylist;
	SteamID mySteamID;
	int teamNumber, currentTab;
	bool playlistMenuOpened = false;

	std::string GetMenuName() override { return pluginWindow->GetMenuName(); }
	std::string GetMenuTitle() override { return pluginWindow->GetMenuTitle(); }

	bool ShouldBlockInput() override { return pluginWindow->ShouldBlockInput(); }
	bool IsActiveOverlay() override { return pluginWindow->IsActiveOverlay(); }

	void SetImGuiContext(uintptr_t ctx) override { pluginWindow->SetImGuiContext(ctx); }

	std::shared_ptr<GameWrapper> getWrapper() { return gameWrapper; }

	void OnOpen() override { return pluginWindow->OnOpen(); }
	void OnClose() override { return pluginWindow->OnClose(); }
	void Render() override { return pluginWindow->Render(); }
	void RenderCanvas(CanvasWrapper canvas);

public:
	std::function<void(std::string)> log;
	std::function<void* ()> getGuiStyle;

	virtual void onLoad();
	virtual void onUnload();


	bool isReady();
	int getWins(int playlist);
	int getLosses(int playlist);
	int getStreak(int playlist);
	float getMMRDelta(int playlist);
	float getMMR(int playlist);

	void setPlaylistMenuOpened(std::string eventName);
	void setMatchmakingViewTab(ServerWrapper caller, void* params, std::string eventName);

	void updateCurrentPlaylist(std::string eventName);
	void updateSteamID();
	void updateSkillData(std::string eventName);

	void drawPlaylist(CanvasWrapper&);

	void reset();
};