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

#include "SessionStats.h"
#include "SessionStatsWindow.h"
#include "utils/parser.h"
#include "bakkesmod/wrappers/GameEvent/ServerWrapper.h"
#include "bakkesmod/wrappers/MMRWrapper.h"
#include "bakkesmod/wrappers/GuiManagerWrapper.h"
#include "bakkesmod/wrappers/gamewrapper.h"

#include <string>
#include <sstream>
#include <map>
#include <set>
#include <fstream>
#include <math.h>

#define DEBUG

std::string UPDATE_SKILL_DATA = "Function TAGame.GFxData_Playlist_TA.UpdateSkillData";
std::string BEGIN_STATE = "Function GameEvent_TA.Countdown.BeginState";
std::string INIT_PLAYLISTS = "Function TAGame.GFxData_Matchmaking_TA.InitPlaylists";
std::string DESTROY_CLASS = "Function Engine.ScriptGroup_ORS.DestroyClass";
std::string TAB_CHANGED = "Function TAGame.GFxData_Matchmaking_TA.SetMatchmakingViewTab";
std::string FINISH_MATCH = "Function TAGame.GFxShell_TA.LeaveMatch";
std::string BEGIN_SEARCH = "Function OnlineGameMatchmakingBase_X.Searching.BeginState";

BAKKESMOD_PLUGIN(SessionStatsPlugin, "Session Stats", "2.0", 7)

void SessionStatsPlugin::onLoad() {
	pluginWindow = new SessionStatsWindow(this);
	log = std::bind(&CVarManagerWrapper::log, cvarManager, std::placeholders::_1);
	getGuiStyle = std::bind(&GuiManagerWrapper::GetImGuiStyle, gameWrapper->GetGUIManager());

#ifdef DEBUG
	// log startup to console
	std::stringstream ss;
	ss << exports.pluginName << " version: " << exports.pluginVersion;
	cvarManager->log(ss.str());
#endif

	// hook cvars
	cvarManager->registerNotifier("cl_sessionstats_reset", [this](std::vector<string> params) {
		reset();
	}, "Start a fresh stats session", PERMISSION_ALL);

	// init state
	currentPlaylist = -1;
	reset();

	// hook events - still need to handle "rage quit" case
	gameWrapper->HookEvent(BEGIN_STATE, bind(&SessionStatsPlugin::updateCurrentPlaylist, this, std::placeholders::_1));
	gameWrapper->HookEvent(FINISH_MATCH, bind(&SessionStatsPlugin::updateSkillData, this, std::placeholders::_1));
	gameWrapper->HookEvent(BEGIN_SEARCH, bind(&SessionStatsPlugin::updateSkillData, this, std::placeholders::_1));
	gameWrapper->HookEventPost(UPDATE_SKILL_DATA, bind(&SessionStatsPlugin::updateSkillData, this, std::placeholders::_1));

	gameWrapper->HookEvent(INIT_PLAYLISTS, bind(&SessionStatsPlugin::setPlaylistMenuOpened, this, std::placeholders::_1));
	gameWrapper->HookEvent(DESTROY_CLASS, bind(&SessionStatsPlugin::setPlaylistMenuOpened, this, std::placeholders::_1));
	gameWrapper->HookEventWithCaller<ServerWrapper>(TAB_CHANGED,
		bind(&SessionStatsPlugin::setMatchmakingViewTab, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));

	gameWrapper->RegisterDrawable(bind(&SessionStatsPlugin::RenderCanvas, this, std::placeholders::_1));
}

void SessionStatsPlugin::onUnload() {
	gameWrapper->UnregisterDrawables();
	gameWrapper->UnhookEvent(BEGIN_STATE);
	gameWrapper->UnhookEvent(UPDATE_SKILL_DATA);
	gameWrapper->UnhookEvent(BEGIN_SEARCH);
	gameWrapper->UnhookEvent(FINISH_MATCH);
	gameWrapper->UnhookEvent(INIT_PLAYLISTS);
	gameWrapper->UnhookEvent(DESTROY_CLASS);
	gameWrapper->UnhookEvent(TAB_CHANGED);
}

bool SessionStatsPlugin::isReady() {
	return gameWrapper != nullptr;
}

int SessionStatsPlugin::getWins(int playlist) {
	return playlists[playlist].wins;
}

int SessionStatsPlugin::getLosses(int playlist) {
	return playlists[playlist].losses;
}

int SessionStatsPlugin::getStreak(int playlist) {
	return playlists[playlist].streak;
}

float SessionStatsPlugin::getMMRDelta(int playlist) {
	return playlists[playlist].currentMMR - playlists[playlist].initialMMR;
}

float SessionStatsPlugin::getMMR(int playlist) {
	return playlists[playlist].currentMMR;
}

void SessionStatsPlugin::setPlaylistMenuOpened(std::string eventName) {
	if (eventName == INIT_PLAYLISTS)
		playlistMenuOpened = true;
	else if (eventName == DESTROY_CLASS)
		playlistMenuOpened = false;
}

void SessionStatsPlugin::setMatchmakingViewTab(ServerWrapper caller, void* params, std::string eventName) {
	currentTab = *(unsigned char*)params;
	cvarManager->log("Changed tabs to " + std::to_string(currentTab));
}

void SessionStatsPlugin::updateCurrentPlaylist(std::string eventName) {
	if (!gameWrapper->IsInOnlineGame() || gameWrapper->IsInReplay())
		return;

	ServerWrapper sw = gameWrapper->GetOnlineGame();
	if (sw.IsNull() || !sw.IsOnlineMultiplayer())
		return;

	MMRWrapper mw = gameWrapper->GetMMRWrapper();
	// Ensure we're only getting playlists whose stats we're keeping track of
	if (playlists.find(mw.GetCurrentPlaylist()) != playlists.end())
		currentPlaylist = mw.GetCurrentPlaylist();
}

void SessionStatsPlugin::updateSkillData(std::string eventName) {
	mySteamID = { gameWrapper->GetSteamID() };
	MMRWrapper mmrWrapper = gameWrapper->GetMMRWrapper();

	for (int i = NUM_TABS - 1; i >= 0; i--) {
		for (int t = NUM_PLAYLIST - 1; t >= 0; t--) {
			int n = PLAYLISTS[i][t];
			float mmr = mmrWrapper.GetPlayerMMR(mySteamID, n);
			if (playlists[n].initialMMR == -1) {
				// (MMR) Initial, Current, Last, etc...
				cvarManager->log("Initial MMR was -1");
				playlists[n].initialMMR = mmr;
				playlists[n].currentMMR = mmr;
				playlists[n].lastMMR = mmr;
				continue;
			}

			playlists[n].currentMMR = mmr;
			if (playlists[n].currentMMR > playlists[n].lastMMR) {
				playlists[n].wins += 1;
				if (playlists[n].streak < 0)
					playlists[n].streak = 1;
				else
					playlists[n].streak++;
			}
			else if (playlists[n].currentMMR < playlists[n].lastMMR) {
				if (playlists[n].streak > 0)
					playlists[n].streak = -1;
				else
					playlists[n].streak--;
				playlists[n].losses += 1;
			}
			playlists[n].lastMMR = mmr;

		}
	}
}

std::stringstream ss;
void SessionStatsPlugin::RenderCanvas(CanvasWrapper canvas) {
	if (!pluginWindow->ShouldDisplayOnMenu() || gameWrapper->IsInGame() || !playlistMenuOpened)
		return;

	for (int i = 0; i < NUM_PLAYLIST; i++) {
		int playlist = PLAYLISTS[currentTab][i];
		float delta = getMMRDelta(playlist);

		ss << std::setprecision(2) << std::fixed << delta;
		std::string str = ss.str();
		int width = str.length() * 4;

		if (delta > 0)
			canvas.SetColor(0, 0xFF, 0, 0xFF);
		else
			canvas.SetColor(0xFF, 0, 0, 0xFF);
		Vector2 pos = { TAB_POS[i].X - width, 350 };
		canvas.SetPosition(pos);
		canvas.DrawString(str, 1.2, 1.2);
		ss.str("");
	}
}

void SessionStatsPlugin::reset() {
	for (int i = NUM_TABS - 1; i >= 0; i--) {
		for (int t = NUM_PLAYLIST - 1; t >= 0; t--) {
			int n = PLAYLISTS[i][t];
			playlists[n].initialMMR = -1;
			playlists[n].lastMMR = 0;
			playlists[n].currentMMR = 0;
			playlists[n].wins = 0;
			playlists[n].losses = 0;
			playlists[n].streak = 0;
		}
	}
}