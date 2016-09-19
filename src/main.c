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

#include "main.h"
#include "init.h"
#include "net.h"
#include "sqlite3.h"

#define APP_DB "ur0:/shell/db/app.db"

int sceAppMgrGetAppParam(char *param);

volatile int dialog_step = DIALOG_STEP_NONE;
static int is_in_archive = 0;

int isInArchive() {
	return is_in_archive;
}

void sql_simple_exec(sqlite3 *db, const char *sql) {
	char *error = NULL;
	int ret = 0;
	ret = sqlite3_exec(db, sql, NULL, NULL, &error);
	if (error) {
		printf("Failed to execute %s: %s\n", sql, error);
		sqlite3_free(error);
		goto fail;
	}
	return;
fail:
	sqlite3_close(db);
}

void do_uri_mod(void) {
	int ret;

	sqlite3 *db;
	ret = sqlite3_open(APP_DB, &db);
	if (ret) {
		printf("Failed to open the database: %s\n", sqlite3_errmsg(db));
	}

	sql_simple_exec(db, "DELETE FROM tbl_uri WHERE titleId='VPKMIRROR'");
	sql_simple_exec(db, "INSERT INTO tbl_uri VALUES ('VPKMIRROR', '1', 'vpk', NULL)");

	sqlite3_close(db);
	db = NULL;

	return;
}

int main() {
	psvDebugScreenInit();
	initSceAppUtil();

	printf("VPK Mirror direct installer v1 created by SMOKE & haxxey\n\n");

	/* grab app param from our custom uri
	   full app param looks like:
	   type=LAUNCH_APP_BY_URI&uri=vpk:install?test
	*/
	char AppParam[1024];
	sceAppMgrGetAppParam(AppParam);

	// checks if argument is present, if not it does the uri mod and opens the website
	int arg_len = strlen(AppParam);
	if (arg_len == 0) {
		do_uri_mod();
		printf("This should now open the browser\nFrom there you can click a link and it will return here\n");
		sceKernelDelayThread(5 * 1000 * 1000); // 5 seconds
		sceAppMgrLaunchAppByUri(0xFFFFF, "http://vpkmirror.com");
		sceKernelDelayThread(10000);
		sceAppMgrLaunchAppByUri(0xFFFFF, "http://vpkmirror.com");
		sceKernelExitProcess(0);
	}

	// argument recieved, init everything else
	netInit();
	httpInit();

	// get the part of the argument that we need
	char *vpk_name;
	vpk_name = strchr(AppParam, '?')+1;

	// create url based off the vpk name
	char *vpk_url = malloc(512 * sizeof(char));
	snprintf(vpk_url, 512, "http://vpkmirror.com/files/vpk/%s", vpk_name);

	// download vpk
	printf("Downloading %s\n", vpk_name);
	char *vpk_path = malloc(512 * sizeof(char));
	snprintf(vpk_path, 512, "ux0:/temp/%s", vpk_name);
	download(vpk_url, vpk_path);

	// install vpk
	printf("Installing %s\n", vpk_name);
	installPackage(vpk_path);

	// cleanup
	sceIoRemove(vpk_path);

	printf("\nAuto exiting in 5 seconds..\n");

	sceKernelDelayThread(5 * 1000 * 1000);

	httpTerm();
	netTerm();
	finishSceAppUtil();

	sceKernelExitProcess(0);
}
