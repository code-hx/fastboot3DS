/*
 *   This file is part of fastboot 3DS
 *   Copyright (C) 2017 derrek, profi200, d0k3
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "arm11/hardware/hid.h"
#include "arm11/hardware/interrupt.h"
#include "arm11/menu/menu_util.h"
#include "arm11/console.h"
#include "arm11/fmt.h"


char* mallocpyString(const char* str)
{
	u32 strsize = strlen(str) + 1;
	char* astr = (char*) malloc(strsize);
	
	if (astr) strncpy(astr, str, strsize);
	return astr;
}

void truncateString(char* dest, const char* orig, int nsize, int tpos)
{
	int osize = strlen(orig);
	
	if (nsize < 0)
	{
		return;
	} else if (nsize <= 3)
	{
		ee_snprintf(dest, nsize, orig);
	} else if (nsize >= osize)
	{
		ee_snprintf(dest, nsize + 1, orig);
	} else
	{
		if (tpos + 3 > nsize) tpos = nsize - 3;
		ee_snprintf(dest, nsize + 1, "%-.*s...%-.*s", tpos, orig, nsize - (3 + tpos), orig + osize - (nsize - (3 + tpos)));
	}
}

void formatBytes(char* str, u64 bytes)
{
	// str should be 32 byte in size, just to be safe
	const char* units[] = {"  Byte", " kiB", " MiB", " GiB"};
	
	if (bytes < 1024)
	{
		ee_snprintf(str, 32, "%llu%s", bytes, units[0]);
	}
	else
	{
		u32 scale = 1;
		u64 bytes100 = (bytes * 100) >> 10;
		for(; (bytes100 >= 1024*100) && (scale < 3); scale++, bytes100 >>= 10);
		ee_snprintf(str, 32, "%llu.%llu%s", bytes100 / 100, (bytes100 % 100) / 10, units[scale]);
	}
}

static const char * convTable[] = {
	"A", "B", "SELECT", "START", "RIGHT", "LEFT",
	"UP", "DOWN", "R", "L", "X", "Y"
};

void keysToString(u32 keys, char* string)
{
	char* ptr = string;
	bool first = true;
	for (u32 i = 0; i < 12; i++)
	{
		if (keys & (1<<i))
		{
			ptr += ee_sprintf(ptr, "%s[%s]", first ? "" : "+", convTable[i]);
			first = false;
		}
	}
	if (first) // backup solution for no buttons
		ee_sprintf(ptr, "(no buttons)");
}

u32 stringGetHeight(const char* str)
{
	u32 height = 1;
	for (char* lf = strchr(str, '\n'); (lf != NULL); lf = strchr(lf + 1, '\n'))
		height++;
	return height;
}

u32 stringGetWidth(const char* str)
{
	u32 width = 0;
	char* old_lf = (char*) str;
	char* str_end = (char*) str + strlen(str);
	for (char* lf = strchr(str, '\n'); lf != NULL; lf = strchr(lf + 1, '\n')) {
		if ((u32) (lf - old_lf) > width) width = lf - old_lf;
		old_lf = lf;
	}
	if ((u32) (str_end - old_lf) > width)
		width = str_end - old_lf;
	return width;
}

void stringWordWrap(char* str, int llen)
{
	char* last_brk = str - 1;
	char* last_spc = str - 1;
	for (char* str_ptr = str;; str_ptr++) {
		if (!*str_ptr || (*str_ptr == ' ')) { // on space or string_end
			if (str_ptr - last_brk > llen) { // if maximum line lenght is exceeded
				if (last_spc > last_brk) { // put a line_brk at the last space
					*last_spc = '\n';
					last_brk = last_spc;
					last_spc = str_ptr;
				} else if (*str_ptr) { // if we have no applicable space
					*str_ptr = '\n';
					last_brk = str_ptr;
				}
			} else if (*str_ptr) last_spc = str_ptr;
		} else if (*str_ptr == '\n') last_brk = str_ptr;
		if (!*str_ptr) break;
	}
}

u32 ee_printf_line_center(const char *const fmt, ...)
{
	char buf[64];
	va_list args;
	va_start(args, fmt);
	ee_vsnprintf(buf, 64, fmt, args);
	va_end(args);
	
	PrintConsole* con = consoleGet();
	int pad = (con->consoleWidth - strlen(buf)) / 2;
	if (pad < 0) pad = 0;
	con->cursorX = 0;
	
	return ee_printf("%*.*s%s\n", pad, pad, "", buf);
}

// only intended to be ran when the shell is closed
void sleepmode(void)
{
	GFX_enterLowPowerState();
	do
	{
		__wfi();
		hidScanInput();
	} while(!(hidKeysUp() & KEY_SHELL));
	GFX_returnFromLowPowerState();
}

void updateScreens(void)
{
	GX_textureCopy((u64*)RENDERBUF_TOP, 0, (u64*)GFX_getFramebuffer(SCREEN_TOP),
				   0, SCREEN_SIZE_TOP + SCREEN_SIZE_SUB);
	GFX_swapFramebufs();
	GFX_waitForEvent(GFX_EVENT_PDC0, true); // VBlank
}

void outputEndWait(void)
{
	u32 kDown = 0;
	
	do
	{
		GFX_waitForEvent(GFX_EVENT_PDC0, true);
		
		if(hidGetPowerButton(false)) // handle power button
			break;
		
		hidScanInput();
		kDown = hidKeysDown();
		if (kDown & (KEY_SHELL)) sleepmode();
	}
	while (!(kDown & (KEY_B|KEY_HOME)));
}
