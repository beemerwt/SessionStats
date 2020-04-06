#pragma once
// Playlist IDs
const int RANKED_STANDARD = 13;
const int RANKED_DOUBLES = 11;
const int RANKED_SOLO_DUEL = 10;
const int RANKED_SOLO_STANDARD = 12;

const int RUMBLE = 28;
const int DROPSHOT = 29;
const int HOOPS = 27;
const int SNOW_DAY = 30;

const int NUM_RANKED_MODES = 8;
const int RANKED_PLAYLIST[NUM_RANKED_MODES] = {
	RANKED_STANDARD,
	RANKED_DOUBLES,
	RANKED_SOLO_DUEL,
	RANKED_SOLO_STANDARD,
	RUMBLE,
	DROPSHOT,
	HOOPS,
	SNOW_DAY
};

const int TAB_PLAYLISTS[3][4] = {
	{ 0, 0, 0, 0 },
	{ RANKED_STANDARD, RANKED_DOUBLES, RANKED_SOLO_DUEL, RANKED_SOLO_STANDARD },
	{ RUMBLE, DROPSHOT, HOOPS, SNOW_DAY }
};

// 1080p Resolution
const Vector2 TAB_POS[4] = {
	{ 567, 350 },
	{ 828, 350 },
	{ 1089, 350 },
	{ 1350, 350 }
};