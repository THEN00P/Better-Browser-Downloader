/*
	VPKMirror
	Copyright (C) 2016, SMOKE

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef __MAIN_H__
#define __MAIN_H__

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <psp2/appmgr.h>
#include <psp2/apputil.h>
#include <psp2/ctrl.h>
#include <psp2/display.h>
#include <psp2/ime_dialog.h>
#include <psp2/message_dialog.h>
#include <psp2/io/dirent.h>
#include <psp2/io/fcntl.h>
#include <psp2/io/stat.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/kernel/threadmgr.h>
#include <psp2/rtc.h>
#include <psp2/sysmodule.h>
#include <vita2d.h>

#include "graphics.h"

#define printf psvDebugScreenPrintf

#define SCE_ERROR_ERRNO_EEXIST 0x80010011
#define SCE_ERROR_ERRNO_ENODEV 0x80010013

#define BIG_BUFFER_SIZE 16 * 1024 * 1024

// Max entries
#define MAX_POSITION 16
#define MAX_ENTRIES 17

#define TEXT_COLOR 0xFFFFFFFF // white
#define TEXT_FOCUS_COLOR 0xFF00FF00 // green

// Font
#define FONT_SIZE 1.0f
#define FONT_X_SPACE 15.0f
#define FONT_Y_SPACE 23.0f

// Main
#define SHELL_MARGIN_X 20.0f
#define SHELL_MARGIN_Y 18.0f

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

#define START_Y (SHELL_MARGIN_Y + 3.0f * FONT_Y_SPACE)

#define pgf_draw_textf(x, y, color, scale, ...) \
	vita2d_pgf_draw_textf(font, x, (y) + 20, color, scale, __VA_ARGS__)

extern vita2d_pgf *font;

#define BACKGROUND_COLOR 0xFF000000

#define SCE_SYSMODULE_PROMOTER_UTIL 0x80000024

extern volatile int dialog_step;

enum DialogSteps {
	DIALOG_STEP_NONE,

	DIALOG_STEP_CANCELLED,

	DIALOG_STEP_ERROR,
	DIALOG_STEP_INFO,
	DIALOG_STEP_SYSTEM,

	DIALOG_STEP_COPYING,
	DIALOG_STEP_COPIED,
	DIALOG_STEP_MOVED,
	DIALOG_STEP_PASTE,

	DIALOG_STEP_DELETED,

	DIALOG_STEP_INSTALL_QUESTION,
	DIALOG_STEP_INSTALL_CONFIRMED,
	DIALOG_STEP_INSTALL_WARNING,
	DIALOG_STEP_INSTALL_WARNING_AGREED,
	DIALOG_STEP_INSTALLING,
	DIALOG_STEP_INSTALLED,

	DIALOG_STEP_EXTRACTING,
	DIALOG_STEP_EXTRACTED,
};

#endif
