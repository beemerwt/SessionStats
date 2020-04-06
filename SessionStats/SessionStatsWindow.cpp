#include "SessionStatsWindow.h"
#include "SessionStats.h"
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "bakkesmod/wrappers/GuiManagerWrapper.h"
#include <map>

// Dropdown menu
static const char* currentPlaylistItem = NULL;
const char* playlistItems[] = {
	"Standard",
	"Doubles",
	"Solo Duel",
	"Solo Standard",
	"Rumble",
	"Dropshot",
	"Hoops",
	"Snow Day"
};

int playlistItemIds[] = {
	RANKED_STANDARD,
	RANKED_DOUBLES,
	RANKED_SOLO_DUEL,
	RANKED_SOLO_STANDARD,
	RUMBLE,
	DROPSHOT,
	HOOPS,
	SNOW_DAY
};

SessionStatsWindow::SessionStatsWindow(SessionStatsPlugin* parent) : parent(parent) {
	currentPlaylistItem = playlistItems[0];
	currentPlaylist = playlistItemIds[0];
}

void SessionStatsWindow::SetImGuiContext(uintptr_t ctx) {
	ImGui::SetCurrentContext(reinterpret_cast<ImGuiContext*>(ctx));
}

std::string SessionStatsWindow::GetMenuName() {
	return "session_stats";
}

std::string SessionStatsWindow::GetMenuTitle() {
	return "Session Stats";
}

bool SessionStatsWindow::ShouldDisplayOnMenu() {
	return displayOnMenu;
}

bool SessionStatsWindow::ShouldBlockInput() {
	return block_input;
}

bool SessionStatsWindow::IsActiveOverlay() {
	return isWindowOpen;
}

void SessionStatsWindow::OnOpen() { }

void SessionStatsWindow::OnClose() { }

void SessionStatsWindow::Render() {
	if (!ImGui::Begin(GetMenuTitle().c_str(), &isWindowOpen, ImGuiWindowFlags_None)) {
		// Early out if the window is collapsed, as an optimization.
		block_input = ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard;
		ImGui::End();
		return;
	}

	// Make style consistent with BM
	ImGuiStyle& style = ImGui::GetStyle();
	if (parent->isReady()) {
		auto bm_style_ptr = parent->getGuiStyle();
		if (bm_style_ptr != nullptr) {
			style = *(ImGuiStyle*)bm_style_ptr;
		}
		else {
			parent->log("bm style ptr was null!!");
		}
	}

	if (ImGui::BeginTabBar("SessionStatsTabs")) {
		if (ImGui::BeginTabItem("Stats")) {
			DisplaySessionStats(style);
			ImGui::EndTabItem();
		}

		if (ImGui::BeginTabItem("Settings")) {
			DisplaySessionSettings(style);
			ImGui::EndTabItem();
		}

		ImGui::EndTabBar();
	}

	ImGui::End();
}

void SessionStatsWindow::DisplaySessionStats(ImGuiStyle& style) {
	int w = 0, l = 0, s = 0;
	float mmrDelta = 0.0, mmr = 0.0;
	if (parent->isReady() && currentPlaylist != 0) {
		w = parent->getWins(currentPlaylist);
		l = parent->getLosses(currentPlaylist);
		s = parent->getStreak(currentPlaylist);
		mmr = parent->getMMR(currentPlaylist);
		mmrDelta = parent->getMMRDelta(currentPlaylist);
	}

	float width = ImGui::CalcItemWidth();
	ImGui::PushItemWidth(width);
	if (ImGui::BeginCombo("##combo", currentPlaylistItem)) {
		for (int n = 0; n < IM_ARRAYSIZE(playlistItems); n++) {
			bool isSelected = (currentPlaylistItem == playlistItems[n]);
			if (ImGui::Selectable(playlistItems[n], isSelected)) {
				currentPlaylistItem = playlistItems[n];
				currentPlaylist = playlistItemIds[n];
			}
			if (isSelected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndCombo();
	}
	ImGui::PopItemWidth();

	ImGui::Text("MMR: %.2f", mmr);
	ImGui::Text("Wins: %d", w);
	ImGui::Text("Losses: %d", l);
	ImGui::Text("Streak: %d", s);
	ImGui::Text("MMR +/-: %.2f", mmrDelta);
}

void SessionStatsWindow::DisplaySessionSettings(ImGuiStyle& style) {
	ImGui::Checkbox("Display on menu", &displayOnMenu);
	if (ImGui::Button("Reset")) {
		parent->reset();
	}
}