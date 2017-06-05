// Copyright 2013 Normmatt
// Licensed under GPLv2 or any later version
// Refer to the license.txt file included.

#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#include "font.h"
#include "draw.h"
#include "fs.h"

static u8 *top_screen0 = NULL;
static u8 *top_screen1 = NULL;
static u8 *bot_screen0 = NULL;
static u8 *bot_screen1 = NULL;

static char debugstr[DBG_N_CHARS_X * DBG_N_CHARS_Y] = { 0 };
static u32 debugcol[DBG_N_CHARS_Y] = { DBG_COLOR_FONT };

void InitScreenFbs(int argc, char *argv[])
{
	if (argc >= 2) {
		/* newer entrypoints */
		struct {
			u8 *top_left;
			u8 *top_right;
			u8 *bottom;
		} *fb = (void *)argv[1];
		top_screen0 = fb[0].top_left;
		top_screen1 = fb[1].top_left;
		bot_screen0 = fb[0].bottom;
		bot_screen1 = fb[1].bottom;
	} else {
		/* outdated entrypoints */
		top_screen0 = (u8 *)(*(u32 *)0x23FFFE00);
		top_screen1 = (u8 *)(*(u32 *)0x23FFFE00);
		bot_screen0 = (u8 *)(*(u32 *)0x23FFFE08);
		bot_screen1 = (u8 *)(*(u32 *)0x23FFFE08);
	}
}

void ClearScreen(u8 *screen, int width, int color)
{
	if (color == COLOR_TRANSPARENT)
		color = COLOR_BLACK;

	for (int i = 0; i < (width * SCREEN_HEIGHT); i++) {
		*(screen++) = color >> 16;      // B
		*(screen++) = color >> 8;       // G
		*(screen++) = color & 0xFF;     // R
	}
}

void ClearScreenFull(bool clear_top, bool clear_bottom)
{
	if (clear_top) {
		ClearScreen(top_screen0, SCREEN_WIDTH_TOP, STD_COLOR_BG);
		ClearScreen(top_screen1, SCREEN_WIDTH_TOP, STD_COLOR_BG);
	}
	if (clear_bottom) {
		ClearScreen(bot_screen0, SCREEN_WIDTH_BOT, STD_COLOR_BG);
		ClearScreen(bot_screen1, SCREEN_WIDTH_BOT, STD_COLOR_BG);
	}
}

void DrawCharacter(u8 *screen, int character, int x, int y, int color, int bgcolor)
{
	for (int yy = 0; yy < 8; yy++) {
		int xDisplacement = (x * BYTES_PER_PIXEL * SCREEN_HEIGHT);
		int yDisplacement = ((SCREEN_HEIGHT - (y + yy) - 1) * BYTES_PER_PIXEL);
		u8 *screenPos = screen + xDisplacement + yDisplacement;

		u8 charPos = font[character * 8 + yy];
		for (int xx = 7; xx >= 0; xx--) {
			if ((charPos >> xx) & 1) {
				*(screenPos + 0) = color >> 16;         // B
				*(screenPos + 1) = color >> 8;          // G
				*(screenPos + 2) = color & 0xFF;        // R
			} else if (bgcolor != COLOR_TRANSPARENT) {
				*(screenPos + 0) = bgcolor >> 16;       // B
				*(screenPos + 1) = bgcolor >> 8;        // G
				*(screenPos + 2) = bgcolor & 0xFF;      // R
			}
			screenPos += BYTES_PER_PIXEL * SCREEN_HEIGHT;
		}
	}
}

void DrawString(u8 *screen, const char *str, int x, int y, int color, int bgcolor)
{
	for (size_t i = 0; i < strlen(str); i++)
		DrawCharacter(screen, str[i], x + i * 8, y, color, bgcolor);
}

void DrawStringF(int x, int y, bool use_top, const char *format, ...)
{
	char str[512] = { 0 }; // 512 should be more than enough
	va_list va;

	va_start(va, format);
	vsnprintf(str, 512, format, va);
	va_end(va);

	for (char *text = strtok(str, "\n"); text != NULL; text = strtok(NULL, "\n"), y += 10) {
		if (use_top) {
			DrawString(top_screen0, text, x, y, STD_COLOR_FONT, STD_COLOR_BG);
			DrawString(top_screen1, text, x, y, STD_COLOR_FONT, STD_COLOR_BG);
		} else {
			DrawString(bot_screen0, text, x, y, STD_COLOR_FONT, STD_COLOR_BG);
			DrawString(bot_screen1, text, x, y, STD_COLOR_FONT, STD_COLOR_BG);
		}
	}
}

void DebugClear()
{
	memset(debugstr, 0x00, DBG_N_CHARS_X * DBG_N_CHARS_Y);
	for (u32 y = 0; y < DBG_N_CHARS_Y; y++)
		debugcol[y] = DBG_COLOR_FONT;
	ClearScreen(top_screen0, SCREEN_WIDTH_TOP, DBG_COLOR_BG);
	ClearScreen(top_screen1, SCREEN_WIDTH_TOP, DBG_COLOR_BG);
}

void DebugSet(const char **strs)
{
	if (strs != NULL) {
		for (int y = 0; y < DBG_N_CHARS_Y; y++) {
			int pos_dbgstr = DBG_N_CHARS_X * (DBG_N_CHARS_Y - 1 - y);
			snprintf(debugstr + pos_dbgstr, DBG_N_CHARS_X, "%-*.*s", DBG_N_CHARS_X - 1, DBG_N_CHARS_X - 1, strs[y]);
			debugcol[y] = DBG_COLOR_FONT;
		}
	}

	int pos_y = DBG_START_Y;
	u32 *col = debugcol + (DBG_N_CHARS_Y - 1);
	for (char *str = debugstr + (DBG_N_CHARS_X * (DBG_N_CHARS_Y - 1)); str >= debugstr; str -= DBG_N_CHARS_X, col--) {
		if (*str != '\0') {
			DrawString(top_screen0, str, DBG_START_X, pos_y, *col, DBG_COLOR_BG);
			DrawString(top_screen1, str, DBG_START_X, pos_y, *col, DBG_COLOR_BG);
			pos_y += DBG_STEP_Y;
		}
	}
}

void DebugColor(u32 color, const char *format, ...)
{
	static bool adv_output = true;
	char tempstr[128] = { 0 }; // 128 instead of DBG_N_CHARS_X for log file
	va_list va;

	va_start(va, format);
	vsnprintf(tempstr, 128, format, va);
	va_end(va);

	if (adv_output) {
		memmove(debugstr + DBG_N_CHARS_X, debugstr, DBG_N_CHARS_X * (DBG_N_CHARS_Y - 1));
		memmove(debugcol + 1, debugcol, (DBG_N_CHARS_Y - 1) * sizeof(u32));
	} else {
		adv_output = true;
	}

	*debugcol = color;
	if (*tempstr != '\r') { // not a good way of doing this - improve this later
		snprintf(debugstr, DBG_N_CHARS_X, "%-*.*s", DBG_N_CHARS_X - 1, DBG_N_CHARS_X - 1, tempstr);
	} else {
		snprintf(debugstr, DBG_N_CHARS_X, "%-*.*s", DBG_N_CHARS_X - 1, DBG_N_CHARS_X - 1, tempstr + 1);
		adv_output = false;
	}

	DebugSet(NULL);
}

void Debug(const char *format, ...)
{
	char tempstr[128] = { 0 }; // 128 instead of DBG_N_CHARS_X for log file
	va_list va;

	va_start(va, format);
	vsnprintf(tempstr, 128, format, va);
	DebugColor(DBG_COLOR_FONT, tempstr);
	va_end(va);
}

void ShowProgress(u64 current, u64 total)
{
	const u32 progX = SCREEN_WIDTH_TOP - 40;
	const u32 progY = SCREEN_HEIGHT - 20;

	if (total > 0) {
		char progStr[8];
		snprintf(progStr, 8, "%3llu%%", (current * 100) / total);
		DrawString(top_screen0, progStr, progX, progY, DBG_COLOR_FONT, DBG_COLOR_BG);
		DrawString(top_screen1, progStr, progX, progY, DBG_COLOR_FONT, DBG_COLOR_BG);
	} else {
		DrawString(top_screen0, "    ", progX, progY, DBG_COLOR_FONT, DBG_COLOR_BG);
		DrawString(top_screen1, "    ", progX, progY, DBG_COLOR_FONT, DBG_COLOR_BG);
	}
}
